/*
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of NXP Semiconductor, Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file    Final_Project.c
 * @brief   Application entry point.
 */
#include <actuators.h>
#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "LPC804.h"
#include "fsl_debug_console.h"
#include "pwm.h"
#include "ultrasonic.h"
#include "lcd_display.h"
#include "lcd_driver.h"
#include "ir_buttons.h"
#include "main.h"
#include "ir.h"
#include "recording.h"
/* TODO: insert other definitions and declarations here. */

/*
 * @brief   Application entry point.
 */

int speed_limit = 100;
volatile int timeout_flag = 0;
int mode = -1;
int prev_mode = -1;
static int prev_us_state = 0;
static int curr_us_state = 0;
int us_mode = US_GEST;
int lcd_lock = 0;
int servo_angle = 0;
long time_ticks = 0;
int servo_ticks = 0;
//void lcd_write_str(char * str) {
//	//Write only the string in the line the cursor is on
//	int i = 0;
//	for (i = 0; i < strlen(str); i++) {
//		lcd_write_char(str[i]);
//	}
//
//	if (i > _lcd_numlines) {
//		for (int j = i; j > _lcd_numlines;j++) {
//			lcd_write_char(' ');
//		}
//	}
//}
void main(void) {
	//char start_str[] = "Welcome to reee!";
	//char init_msg[] = "Initializing...";
	//BOARD_InitDebugConsole();
	//Clock select
	SYSCON->MAINCLKSEL = (0x0 << SYSCON_MAINCLKSEL_SEL_SHIFT);
	SYSCON->MAINCLKUEN &= ~(0x1);
	SYSCON->MAINCLKUEN |= 0x1;
	BOARD_BootClockFRO30M();
	//15MHz
	lcd_begin(16, 2, LCD_5x8DOTS);
	lcd_write_str("Welcome to SMB!");
	lcd_setCursor(0, 1);
	lcd_write_str("Initializing...");
	init_motors();
	init_pwm();
	init_ir();
	init_timeout();
	init_ultrasonic();
	init_linetrack();
	SysTick_Config(900000);

	//lcd_clear();

	enable_gest();
	//Wait for 2 seconds
	move_servo(-45);
	delayMicroseconds(500000);
	move_servo(45);
	delayMicroseconds(500000);
	move_servo(0);
	delayMicroseconds(15000);
	change_mode(IDLE);
	lcd_setCursor(0, 1);
	lcd_write_str("Init complete  ");
	en_timeout();
	while (1) {
		//printf("We looping bois\n");
		//pwm(SERVO_PWM_REG, duty_cycle);
		//printf("executing...\n");
		//lcd_clear();
		//lcd_write_str("we DONE...");
		curr_us_state = get_us_state();
		if (curr_us_state != prev_us_state) {
			if (curr_us_state == US_STATE_GEST_STOP) {
				prev_us_state = curr_us_state;
				//printf("Forcestop\n");
				//lcd_clear();
				stop();
				//en_timeout();
			} else if (curr_us_state == US_STATE_GEST_START) {
				prev_us_state = curr_us_state;
				//lcd_clear();
				lcd_setCursor(0, 1);
				lcd_write_str("Gesture: Start    ");
				delayMicroseconds(200000);
				//en_timeout();

				start();
				//enable_gest();
			} else if (curr_us_state == US_STATE_GEST_INGEST) {
				prev_us_state = curr_us_state;
			}

			int button = get_ir_btn();
			if (button > 0) {
				//printf("Button %d\n: ", button);
				if (!lcd_lock) {
					//lcd_display_button(button);
					//timeout_flag = 1;
				}
				process_button(button);
				reset_ir_state();
			}
			switch (mode) {
			case OBS_AVOID:
				obs_avoid();
				break;
			case LINE_TRACK:
				line_track();
			}

			//flag = 0;
			//print_us_hist();
			//printf("%d\n", get_distance());
			//flag = 0;

			//i++ ;
			/* 'Dummy' NOP to allow source level single stepping of
			 tight while() loop */
		}
		__asm volatile ("nop");
	}
}

void line_track() {
	if (mode == LINE_TRACK) {
		//drive(0,0);
		//Active low pins
		int right = GPIO->B[0][LINE_TRACK_RIGHT];
		int middle = GPIO->B[0][LINE_TRACK_MIDDLE];
		int left = GPIO->B[0][LINE_TRACK_LEFT];
		if (!middle) {
			lcd_drive(FORWARD_LEFT, FORWARD_RIGHT);
			delayMicroseconds(700000);
			return;
		}

		if (!right) {
			lcd_drive(RIGHT_LEFT, RIGHT_RIGHT);
			delayMicroseconds(700000);
			return;
		}
		if (!left) {
			lcd_drive(LEFT_LEFT, RIGHT_RIGHT);
			delayMicroseconds(700000);
			return;
		}
		//If we haven't returned yet, we can't find the line!
		lcd_setCursor(0, 1);
		lcd_write_str("Can't find line!");
		en_timeout();
	}
}
void obs_avoid() {
	if (mode == OBS_AVOID) {
		//First, turn all motors off
		drive(0, 0);
		//Turn ultrasonic sensor mode to manual
		us_mode = US_MAN;
		lcd_setCursor(0, 1);
		lcd_write_str("Checking for obstacles..");
		//Get distance forward
		move_servo(0);
		delayMicroseconds(300000);
		int dist_f = get_distance();
		if (dist_f < 10) {
			stop();
			lcd_setCursor(0, 1);
			lcd_write_str("Gesture: Stop   ");
			change_mode(stop);
			return;
		} else if (dist_f > OBS_THRESHOLD) {
			lcd_drive(FORWARD_LEFT, FORWARD_RIGHT);
			while (timeout_flag)
				;
			return;
		}

		move_servo(90);
		lcd_setCursor(0, 1);
		lcd_write_str("Obs Detected!     ");
		delayMicroseconds(1000000);
		int dist_r = get_distance();
		delayMicroseconds(100000);
		move_servo(-90);
		delayMicroseconds(1000000);
		int dist_l = get_distance();

		if ((dist_r < OBS_THRESHOLD) && (dist_l < OBS_THRESHOLD)) {
			lcd_setCursor(0, 0);
			lcd_write_str("Obstacle:F, R, L  ");
			lcd_setCursor(0, 1);
			lcd_write_str("Unable to avoid!  ");
			delayMicroseconds(500000);
			return;
		}
		if (dist_r > OBS_THRESHOLD) {
			lcd_drive(RIGHT_LEFT, RIGHT_RIGHT);
			while (timeout_flag)
				;
			return;
		}
		if (dist_l > OBS_THRESHOLD) {
			lcd_drive(LEFT_LEFT, LEFT_RIGHT);
			while (timeout_flag)
				;
			return;
		}
		//We should not be here, if we are, just change mode to stop
		change_mode(STOP);
	}
}
void init_timeout() {
	//Enable timeout -- oneshot mode
	MRT0->CHANNEL[MRT_TIMEOUT].CTRL = (1 << MRT_CHANNEL_CTRL_MODE_SHIFT // Bit 1 is 0
	| MRT_CHANNEL_CTRL_INTEN_MASK);
}

void en_timeout() {
	MRT0->CHANNEL[MRT_TIMEOUT].INTVAL = MRT_TIMEOUT_RELOAD_VAL;
	timeout_flag = 1;
}

void start() {
	if (mode == IDLE) {
		lcd_setCursor(0, 1);
		delayMicroseconds(500000);
		lcd_write_str("Select a mode  ");
	} else {
		int p_m = prev_mode;
		change_mode(prev_mode);
		if (prev_mode == REPLAY) {
			replay_record(1);
		} else if (prev_mode == REPLAY_MIRROR) {
			replay_record(-1);
		}
	}
	en_timeout();
}
void stop() {
	drive(0, 0);
	lcd_setCursor(0, 1);
	lcd_write_str("Gesture: Stop   ");
	delayMicroseconds(300000);
	if (mode == IDLE) {
		lcd_setCursor(0, 1);
		//Wait ~0.5s
		delayMicroseconds(500000);
		lcd_write_str("Already stopped!   ");
		en_timeout();
	} else {
		change_mode(STOP);
	}

}
void change_mode(int new_mode) {
	lcd_setCursor(0, 1);
	lcd_write_str("                          ");
	if (mode != new_mode) {
		lcd_setCursor(0, 0);
		if (new_mode == IDLE) {
			lcd_write_str("Mode: Idle      ");
		} else if (new_mode == LINE_TRACK) {
			lcd_write_str("Mode: Line Track");
		} else if (new_mode == RECORD) {
			lcd_write_str("Mode: Record    ");
		} else if (new_mode == REPLAY) {
			lcd_write_str("Mode: Replay    ");
			lcd_setCursor(0, 1);
			lcd_write_str("                       ");
			lcd_setCursor(0, 1);
			lcd_write_str("OK/Gest to START      ");
		} else if (new_mode == OBS_AVOID) {
			lcd_write_str("Mode: Obst Avoid");
		} else if (new_mode == REMOTE_CONTROL) {
			lcd_write_str("Mode: RC        ");
		} else if (new_mode == STOP) {
			lcd_write_str("Status: Stopped  ");
			lcd_setCursor(0, 1);
			lcd_write_str("Prev mode:");
			switch (mode) {
			case (IDLE):
				lcd_write_str("IDLE  ");
				break;
			case (REMOTE_CONTROL):
				lcd_write_str("RCTRL  ");
				break;
			case (LINE_TRACK):
				lcd_write_str("LTRK  ");
				break;
			case (RECORD):
				lcd_write_str("RECORD ");
				break;
			case (REPLAY):
				lcd_write_str("REPLAY ");
				break;
			case (REPLAY_MIRROR):
				lcd_write_str("RPLY MRR");
				break;
			case (OBS_AVOID):
				lcd_write_str("OBS AVD");
				break;
			}
		} else if (new_mode == REPLAY_MIRROR) {
			lcd_write_str("Mode: Replay Mirr    ");
			lcd_setCursor(0, 1);
			lcd_write_str("                       ");
			lcd_setCursor(0, 1);
			lcd_write_str("OK/Gest to START      ");
		}
		prev_mode = mode;
		mode = new_mode;

		if (new_mode != OBS_AVOID) {
			us_mode = US_GEST;
		}
	}
}

void process_button(int button) {
	if (mode == REMOTE_CONTROL) {
		switch (button) {
		case BTN_UP:
			//drive forward
			lcd_drive(FORWARD_LEFT, FORWARD_RIGHT);
			break;
		case BTN_RIGHT:
			//drive right
			lcd_drive(RIGHT_LEFT, RIGHT_RIGHT);
			//lcd_drive(0, 100);
			break;
		case BTN_LEFT:
			//drive left
			lcd_drive(LEFT_LEFT, LEFT_RIGHT);
			break;
		case BTN_DOWN:
			lcd_drive(BACKWARD_LEFT, BACKWARD_RIGHT);
			//drive backward
			break;
		case BTN_OK:
			if (mode == STOP) {
			} else {
				drive(0, 0);
				lcd_setCursor(0, 1);
				lcd_write_str("STOPPING        ");
				//change_mode(STOP);
			}
			break;
		}
		if (timeout_flag) {
			MRT0->CHANNEL[MRT_TIMEOUT].INTVAL = MRT_TIMEOUT_RELOAD_VAL;
		}
	}
	//Use buttons to change the mode
	switch (button) {
	case BTN_0:
		change_mode(IDLE);
		en_timeout();
		return;
	case BTN_1:
		change_mode(REMOTE_CONTROL);
		return;
	case BTN_2:
		change_mode(RECORD);
		return;
	case BTN_3:
		change_mode(REPLAY);
		lcd_setCursor(0, 1);
		lcd_write_str("OK/Gest to START      ");
		return;
	case BTN_4:
		change_mode(LINE_TRACK);
		return;
	case BTN_5:
		change_mode(OBS_AVOID);
		return;
	case BTN_6:
		change_mode(REPLAY_MIRROR);
		en_timeout();
		lcd_setCursor(0, 1);
		lcd_write_str("OK/Gest to START      ");
		return;
	case BTN_AST:
		if (mode == RECORD) {
			disp_record();
		} else {
			change_speed(-1);
		}
		return;
	case BTN_PND:
		if (mode == RECORD) {
			delete_record();
		} else {
			change_speed(1);
		}
		return;
	case BTN_7:
		servo_angle = servo_angle - 45;
		move_servo(servo_angle);
		lcd_setCursor(0, 1);
		lcd_write_str("Pan servo left  ");
		en_timeout();
		return;
	case BTN_8:
		servo_angle = 0;
		move_servo(servo_angle);
		lcd_setCursor(0, 1);
		lcd_write_str("Reset servo pos ");
		en_timeout();
		return;
	case BTN_9:
		servo_angle = servo_angle + 45;
		move_servo(servo_angle);
		lcd_setCursor(0, 1);
		lcd_write_str("Pan servo right  ");
		en_timeout();
		return;
	case BTN_OK:
		if (mode == IDLE) {
			long time_elapse = time_ticks / 10;
			lcd_setCursor(0, 0);
			lcd_write_str("                  ");
			lcd_setCursor(0, 0);
			lcd_write_str("Time elapsed:");
			lcd_setCursor(0, 1);
			lcd_write_str("                   ");
			lcd_setCursor(0, 1);
			int dig3 = time_ticks % 10;
			int dig2 = (time_ticks / 10) % 10;
			int dig1 = ((time_ticks / 10) / 10) % 10;
			if (dig1 == 0) {
				if (dig2 == 0) {
					lcd_write_char(dig3 + 48);
				} else {
					lcd_write_char(dig2 + 48);
					lcd_write_char(dig3 + 48);
				}
			} else {
				lcd_write_char(dig1 + 48);
				lcd_write_char(dig2 + 48);
				lcd_write_char(dig3 + 48);
			}
			lcd_write_str(" s");
			en_timeout();
		}
	}
	if (mode == RECORD) {
		add_record_cmd(button);
	} else if (mode == REPLAY) {
		lcd_lock = 1;
		replay_record(1);
		lcd_lock = 0;
	} else if (mode == REPLAY_MIRROR) {
		lcd_lock = 1;
		replay_record(-1);
		lcd_lock = 0;
	}

}
void lcd_drive(int left, int right) {
	lcd_setCursor(0, 1);
	if ((right > 0) && (left > 0)) {
		lcd_write_str("Driving FORWARD     ");
	} else if ((right <= 0) && (left > 0)) {
		lcd_write_str("Driving RIGHT       ");
	} else if ((left <= 0) && (right > 0)) {
		lcd_write_str("Driving LEFT      ");
	} else if ((left < 0) && (right < 0)) {
		lcd_write_str("Driving BACKWARD   ");
	}
	en_timeout();
	drive(left, right);

}
void lcd_dir_char(int button) {
	switch (button) {
	case BTN_UP:
		lcd_write_str("F          ");
		break;
	case BTN_RIGHT:
		lcd_write_str("R           ");
		break;
	case BTN_LEFT:
		lcd_write_str("L            ");
		break;
	case BTN_DOWN:
		lcd_write_str("B            ");
		break;
	case BTN_OK:
		lcd_write_str("            ");
		break;
	}
}

void change_speed(int x) {
	if (x == -1) {
		speed_limit = speed_limit - 25;
		if (speed_limit < 0) {
			speed_limit = 0;
		}
		//Extract digits
		int dig3 = speed_limit % 10;
		int dig2 = (speed_limit / 10) % 10;
		int dig1 = ((speed_limit / 10) / 10) % 10;
		lcd_setCursor(0, 1);
		lcd_write_str("New speed: ");
		if (dig1 == 0) {
			lcd_write_char(dig2 + 48);
			lcd_write_char(dig3 + 48);
		} else {
			lcd_write_char(dig1 + 48);
			lcd_write_char(dig2 + 48);
			lcd_write_char(dig3 + 48);
		}
		lcd_write_str("%");
		delayMicroseconds(500000);
		en_timeout();
	} else if (x == 1) {
		speed_limit = speed_limit + 25;
		if (speed_limit > 100) {
			speed_limit = 100;
		}
		//Extract digits
		int dig3 = speed_limit % 10;
		int dig2 = (speed_limit / 10) % 10;
		int dig1 = ((speed_limit / 10) / 10) % 10;
		lcd_setCursor(0, 1);
		lcd_write_str("New speed: ");
		if (dig1 == 0) {
			lcd_write_char(dig2 + 48);
			lcd_write_char(dig3 + 48);
		} else {
			lcd_write_char(dig1 + 48);
			lcd_write_char(dig2 + 48);
			lcd_write_char(dig3 + 48);
		}
		lcd_write_str("%");
		delayMicroseconds(500000);
	}
}
void init_linetrack() {
	//Set those pins as inputs
	//Enable GPIO clock
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK);
	GPIO->DIRCLR[0] = (1UL << LINE_TRACK_LEFT) | (1UL << LINE_TRACK_RIGHT)
			| (1UL << LINE_TRACK_MIDDLE);
}
void SysTick_Handler(void) {
	//Trigger the ultrasonic sensor
	time_ticks++;
	if (us_mode == US_GEST) {
		if (servo_ticks == 6) {
			trig_gest();
			servo_ticks = 0;
		}
	}
	servo_ticks++;

}
