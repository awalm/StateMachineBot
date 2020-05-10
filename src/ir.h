/*
 * ir.h
 *
 *  Created on: Mar. 30, 2020
 *      Author: masta
 */

#ifndef IR_H_
#define IR_H_

//Move to master decalartions list
#define IR_PIN (9)
#define MRT_IR (1)
#define MRT_POLL_RELOAD (750) //50us, at 15MHz
#define USEC_PER_TICK (50)


//State Machine States
#define IR_STATE_IDLE (0)
#define IR_STATE_START_PULSE (1)
#define IR_STATE_START_SPACE (2)
#define IR_STATE_RECV (3)
#define IR_STATE_FINISH_RECV (4)
#define IR_STATE_DECODING (5)
#define IR_STATE_DECODED (6)
#define IR_STATE_DECODE_ERR (7)
#define IR_STATE_RECV_ERR (8)
#define IR_STATE_ERR (9)
#define IR_STATE_BTN_PRESSED (10)
//Global variables

//Function declarations
int init_ir(void);
void PIN_INT1_IRQHandler();
void calc_us_buf();
void decode();
void print_us_buf();
int get_ir_state();
int get_ir_btn();
void reset_ir_state();
void clear_err();


#define START_PULSE (9000)
#define START_SPACE (4500)
#define BIT_PULSE (560)
#define LOG_LOW_SPACE (560)
#define LOG_HIGH_SPACE (1690)
#define RPT_SPACE (2250)

#define TOLERANCE (25) //Tolerance in percentage
#define LOW_TOL (1.00 - (TOLERANCE/100.00))
#define HIGH_TOL (1.00 + (TOLERANCE/100.00))

#define START_PULSE_TICKS_HIGH (((START_PULSE)*HIGH_TOL)/(USEC_PER_TICK))
#define START_PULSE_TICKS_LOW (((START_PULSE)*LOW_TOL)/(USEC_PER_TICK))

#define START_SPACE_TICKS_HIGH (((START_SPACE)*HIGH_TOL)/(USEC_PER_TICK))
#define START_SPACE_TICKS_LOW (((START_SPACE)*LOW_TOL)/(USEC_PER_TICK))

#define BIT_PULSE_TICKS_HIGH (((BIT_PULSE)*HIGH_TOL)/(USEC_PER_TICK))
#define BIT_PULSE_TICKS_LOW (((BIT_PULSE)*LOW_TOL)/(USEC_PER_TICK))

#define LOG_LOW_TICKS_HIGH (((LOG_LOW_SPACE)*HIGH_TOL)/(USEC_PER_TICK))
#define LOG_LOW_TICKS_LOW (((LOG_LOW_SPACE)*LOW_TOL)/(USEC_PER_TICK))

#define LOG_HIGH_TICKS_HIGH (((LOG_HIGH_SPACE)*HIGH_TOL)/(USEC_PER_TICK))
#define LOG_HIGH_TICKS_LOW (((LOG_HIGH_SPACE)*LOW_TOL)/(USEC_PER_TICK))

#define RPT_TICKS_HIGH (((RPT_SPACE)*HIGH_TOL)/(USEC_PER_TICK))
#define RPT_TICKS_LOW (((RPT_SPACE)*LOW_TOL)/(USEC_PER_TICK))


#endif /* IR_H_ */
