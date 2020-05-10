/*
 * recording.c
 *
 *  Created on: Apr. 8, 2020
 *      Author: masta
 */

#include "LPC804.h"
#include "ir_buttons.h"
#include "recording.h"
#include "main.h"
#include "lcd_driver.h"
#include "actuators.h"
#include "lcd_display.h"

int commands[16];
int cmd_idx = 0;
extern int lcd_lock;
extern volatile int timeout_flag;

void add_record_cmd(int button) {
	lcd_lock = 1;
	int dir = -9;
	switch (button) {
	case BTN_UP:
		dir = DIR_FORWARD;
		break;
	case BTN_RIGHT:
		dir = DIR_RIGHT;
		break;
	case BTN_LEFT:
		dir = DIR_LEFT;
		break;
	case BTN_DOWN:
		dir = DIR_BACKWARD;
		break;
	case BTN_OK:
		dir = DIR_STOP;
		break;
	}
	if (dir == -9) {
		lcd_setCursor(0,1);
		lcd_write_str("Use D-PAD to rec ");
	} else {
		lcd_setCursor(0,0);
		lcd_write_str("                  ");
		lcd_setCursor(0,0);
		lcd_write_str ("Adding: ");
		lcd_dir_str_drv(button);
		lcd_setCursor(0,1);
		//add 48 to convert to ascii
		if (cmd_idx > 16) {
			lcd_write_str("Buffer full!     ");
		} else {
			lcd_write_str ("Size: ");
			if (cmd_idx <= 9) {
				lcd_write_char (cmd_idx + 49);
			} else {
				lcd_write_char (49);
				lcd_write_char((cmd_idx - 10) + 48);
			}
			lcd_write_char('/');
			lcd_write_str("16         ");
			commands[cmd_idx++] = dir;
		}
		delayMicroseconds(1000000);
		lcd_setCursor(0,1);
		disp_record();
	}
	lcd_lock = 0;

}
void lcd_dir_str(int button) {
	switch(button) {
		case BTN_UP:
			lcd_write_str("FORWARD      ");
			break;
		case BTN_RIGHT:
			lcd_write_str("RIGHT        ");
			break;
		case BTN_LEFT:
			lcd_write_str("LEFT         ");
			break;
		case BTN_DOWN:
			lcd_write_str("BACKWARD      ");
			break;
		case BTN_OK:
			lcd_write_str("STOP          ");
			break;
	}
}

void lcd_dir_str_drv(int button) {
	//Return string and drive
	en_timeout();
	switch(button) {
		case BTN_UP:
			lcd_write_str("FORWARD      ");
			drive(FORWARD_LEFT,FORWARD_RIGHT);
			break;
		case BTN_RIGHT:
			lcd_write_str("RIGHT        ");
			drive(RIGHT_LEFT,RIGHT_RIGHT);
			break;
		case BTN_LEFT:
			lcd_write_str("LEFT         ");
			drive(LEFT_LEFT,LEFT_RIGHT);
			break;
		case BTN_DOWN:
			lcd_write_str("BACKWARD      ");
			drive(BACKWARD_LEFT,BACKWARD_RIGHT);
			break;
		case BTN_OK:
			lcd_write_str("STOP          ");
			drive(0,0);
			break;
	}
}

void replay_record(int dir) {
	lcd_lock = 1;
	lcd_setCursor(0, 0);
	if (dir == -1) {
		lcd_write_str("Replay Mirror:       ");
		disp_only_record();
		lcd_setCursor(cmd_idx, 1);
		for (int i = cmd_idx; i >0 ; i--) {
			lcd_setCursor(i - 1, 1);
			lcd_cursor();
			en_timeout();
			drive_dir((-1)*commands[i-1]);
			while(timeout_flag);
			delayMicroseconds(500000);
		}
	} else {
		lcd_write_str("Replaying:          ");
		disp_only_record();
		//lcd_setCursor(0, 1);
		//lcd_blink();
		//lcd_cursor();
		for (int i = 0; i < cmd_idx; i++) {
			lcd_setCursor(i, 1);
			lcd_cursor();
			en_timeout();
			drive_dir(commands[i]);
			while(timeout_flag);
			delayMicroseconds(500000);
			//delayMicroseconds(1000000);
		}
	}
	lcd_noCursor();
	lcd_setCursor(0,1);
	lcd_write_str("                  ");
	lcd_setCursor(0,1);
	lcd_write_str("Completed Replay");
	delayMicroseconds(1000000);
	lcd_setCursor(0,1);
	lcd_write_str("OK/Gest to START      ");
	lcd_lock = 0;
	//en_timeout();
	if (dir == 1) {
		change_mode(REPLAY_MIRROR);
	} else {
		change_mode(REPLAY);
	}

}
void drive_dir(int dir) {
	switch (dir) {
	case DIR_FORWARD:
		drive(FORWARD_LEFT,FORWARD_RIGHT);
		break;
	case DIR_BACKWARD:
		drive(BACKWARD_LEFT, BACKWARD_RIGHT);
		break;
	case DIR_LEFT:
		drive(LEFT_LEFT, LEFT_RIGHT);
		break;
	case DIR_RIGHT:
		drive(RIGHT_LEFT,RIGHT_RIGHT);
		break;
	case DIR_STOP:
		drive(0,0);
		break;
	}
}
void disp_only_record() {
	lcd_setCursor(0,1);
	lcd_write_str("                             ");
	lcd_setCursor(0,1);
	for (int i = 0; i < 16; i++) {
		switch (commands[i]) {
		case DIR_FORWARD:
			lcd_write_str("F");
			break;
		case DIR_BACKWARD:
			lcd_write_str("B");
			break;
		case DIR_RIGHT:
			lcd_write_str("R");
			break;
		case DIR_LEFT:
			lcd_write_str("L");
			break;
		case DIR_STOP:
			lcd_write_str(" ");
			break;
		}
	}
}
void disp_record() {
	lcd_setCursor(0, 0);
	lcd_write_str("                     ");
	lcd_setCursor(0,0);
	lcd_write_str("Recording:           ");
	lcd_setCursor(0, 1);
	lcd_write_str("                     ");
	lcd_setCursor(0, 1);
	if (cmd_idx == 0) {
		lcd_write_str("Empty!        ");
	}

	for (int i = 0; i < 16; i++) {
		switch (commands[i]) {
		case DIR_FORWARD:
			lcd_write_str("F");
			break;
		case DIR_BACKWARD:
			lcd_write_str("B");
			break;
		case DIR_RIGHT:
			lcd_write_str("R");
			break;
		case DIR_LEFT:
			lcd_write_str("L");
			break;
		case DIR_STOP:
			lcd_write_str(" ");
			break;
		}
	}
}

void delete_record() {
	lcd_setCursor(0, 0);
	lcd_write_str("Deleting Record..");
	disp_only_record();
	delayMicroseconds(100000);
	for (int i = 0; i < 16; i++) {
		commands[i] = 0;
	}
	lcd_write_str("Record deleted.");
	disp_only_record();
	delayMicroseconds(200000);
	lcd_setCursor(0,0);
	lcd_write_str("Mode: Record    ");
	cmd_idx = 0;

}

int get_cmd_idx() {
	return cmd_idx;
}
