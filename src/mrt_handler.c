/*
 * mrt_handler.c
 *
 *  Created on: Apr. 8, 2020
 *      Author: masta
 */
#include "LPC804.h"
#include "ultrasonic.h"
#include "ir.h"
#include "main.h"

extern int calc_flag;
extern int num_echos;
extern int curr_pin_state;
extern int prev_pin_state;
extern int prev_state_ticks;
extern int ir_state;
extern int raw_buf[68];
extern int bits_read;
extern int key_repeat;
extern int lcd_lock;
extern int timeout_flag;
//int decode_err = 0;
//int us_buf[68];
////stores how many times the previous state has been repeated
//volatile int prev_state_ticks = 1;
//volatile int curr_pin_state = 0;
//volatile int bits_read = 0;
//volatile int key_repeat = 0;
//volatile int button_pressed;
//volatile int prev_pin_state = 0;
void MRT0_IRQHandler(void) {
	//printf("We polling bois, at time:%u\n", WKT->COUNT);

	if (MRT0->IRQ_FLAG & (1 << MRT_ULTRASONIC)) {
		//printf("U\n");
		if (((GPIO->B[0][ULTRASONIC_ECHO]) & 1) && num_echos <= MAX_ECHOS) {
			//Check if num_echos is greater than 3000
			//printf("echo");
			num_echos++;
		} else {
			//Unblock the flag
			calc_flag = 0;
			//Disable the interrupt
			MRT0->CHANNEL[0].INTVAL = 0;
		}
		MRT0->CHANNEL[MRT_ULTRASONIC].STAT = MRT_CHANNEL_STAT_INTFLAG_MASK;
	} else if (MRT0->IRQ_FLAG & (1 << MRT_IR)) {
		//IR//Clear IRQ
		MRT0->CHANNEL[MRT_IR].STAT = MRT_CHANNEL_STAT_INTFLAG_MASK;
		curr_pin_state = GPIO->B[0][IR_PIN];
		//If the previous pin state is the same as the current pin state, then
		//no state machine transition, as it is the same
		if (bits_read == 66) {
			ir_state = IR_STATE_FINISH_RECV;
			return;
		}
		if (prev_pin_state == curr_pin_state) {
			prev_state_ticks++;
			if (prev_state_ticks > START_PULSE_TICKS_HIGH) {
				ir_state = IR_STATE_ERR;
				//reset_state();
			}
			return;
		} else {
			prev_pin_state = curr_pin_state;
			//Check current state
			if (ir_state == IR_STATE_FINISH_RECV || ir_state == IR_STATE_ERR) {
				MRT0->CHANNEL[MRT_IR].INTVAL = 0;
				NVIC_EnableIRQ(PIN_INT1_IRQn);
				prev_pin_state = 0;
				curr_pin_state = 0;
				prev_state_ticks = 0;
				bits_read = 0;
				return;
			}

			if ((ir_state == IR_STATE_START_PULSE) && (curr_pin_state == 1)) {
				//Check right number of ticks
				if ((prev_state_ticks >= START_PULSE_TICKS_LOW)
						&& (prev_state_ticks <= START_PULSE_TICKS_HIGH)) {
					//Update state
					ir_state = IR_STATE_START_SPACE;
					if (curr_pin_state == 0) {
						raw_buf[bits_read] = (-1) * prev_state_ticks;
					} else if (curr_pin_state == 1) {
						raw_buf[bits_read] = prev_state_ticks;
					} else {
						ir_state = IR_STATE_ERR;
					}
					bits_read++;
				}
			}
			//printf("State: Space start. State change ticks: %d", prev_state_ticks);
			else if ((ir_state == IR_STATE_START_SPACE) && (curr_pin_state == 0)) {
				if ((prev_state_ticks >= START_SPACE_TICKS_LOW)
						&& (prev_state_ticks <= START_SPACE_TICKS_HIGH)) {
					//Update state
					ir_state = IR_STATE_RECV;
					if (curr_pin_state == 0) {
						raw_buf[bits_read] = (-1) * prev_state_ticks;
					} else if (curr_pin_state == 1) {
						raw_buf[bits_read] = prev_state_ticks;
					} else {
						ir_state = IR_STATE_ERR;
					}
					bits_read++;
					//printf("State: Space start. State change ticks: %d", prev_state_ticks);
				} else if ((prev_state_ticks >= RPT_TICKS_LOW)
						&& (prev_state_ticks <= RPT_TICKS_HIGH)) {
					ir_state = IR_STATE_FINISH_RECV;
					key_repeat = 1;
					if (curr_pin_state == 0) {
						raw_buf[bits_read] = (-1) * prev_state_ticks;
					} else if (curr_pin_state == 1) {
						raw_buf[bits_read] = prev_state_ticks;
					} else {
						ir_state = IR_STATE_ERR;
					}
					bits_read++;
					return;
				} else {
					ir_state = IR_STATE_ERR;
				}
				//State machine for after start pulse --> either repeat space or 4.5ms space
//			} else if ((ir_state == STATE_RECV_BIT_PULSE)
//					&& (curr_pin_state == 1)) {
//				if (bits_read > 66) {
//					ir_state = STATE_FINISH_RECV;
//					return;
//				}
//				if ((prev_state_ticks >= BIT_PULSE_TICKS_LOW)
//						&& (prev_state_ticks <= BIT_PULSE_TICKS_HIGH)) {
//					ir_state = STATE_RECV;
//					if (curr_pin_state == 0) {
//						raw_buf[bits_read] = (-1) * prev_state_ticks;
//					} else if (curr_pin_state == 1) {
//						raw_buf[bits_read] = prev_state_ticks;
//					} else {
//						ir_state = STATE_ERR;
//					}
//					bits_read++;
//				} else {
//					ir_state = STATE_ERR;
//				}
			} else if ((ir_state == IR_STATE_RECV)) {
				//If state receive, then receive the message
				//If 33 bits are read, then the message frame is complete
				if (bits_read > 66) {
					ir_state = IR_STATE_FINISH_RECV;
					//decode();
					return;
				}
				//				if ((prev_state_ticks >= LOG_LOW_TICKS_LOW) &&
				//						(prev_state_ticks <= LOG_LOW_TICKS_HIGH)) {
				//ir_state = STATE_RECV_BIT_PULSE;
				if (curr_pin_state == 0) {
					raw_buf[bits_read] = (-1) * prev_state_ticks;
				} else if (curr_pin_state == 1) {
					raw_buf[bits_read] = prev_state_ticks;
				} else {
					ir_state = IR_STATE_ERR;
				}
				bits_read++;
			}
			prev_state_ticks = 1;
		}
	}  else if (MRT0->IRQ_FLAG & (1 << MRT_TIMEOUT)) {
		MRT0->CHANNEL[MRT_TIMEOUT].STAT = MRT_CHANNEL_STAT_INTFLAG_MASK;
		drive(0,0);
		//Clear bottom line if LCD is not locked
		if(!lcd_lock) {
			lcd_setCursor(0,1);
			lcd_write_str("                ");
		}
		timeout_flag = 0;
	}
}

