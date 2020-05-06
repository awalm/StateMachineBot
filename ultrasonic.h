/*
 * ultrasonic.h
 *
 *  Created on: Mar. 31, 2020
 *      Author: masta
 */

#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#define ULTRASONIC_GEST_RELOAD (900000) //15MHz/60ms
#define ULTRASONIC_POLL_RELOAD (90) //poll rate (15MHz/6us = 90)
#define ULTRASONIC_TIMEOUT (3000)
#define ULTRASONIC_TRIG (1)
#define ULTRASONIC_ECHO (4)
#define MAX_ECHOS (4000) //Max is 4m from datasheet
#define MRT_ULTRASONIC (0)

#define GESTURE_HISTORY (5)
#define US_STATE_IDLE (0)
#define US_STATE_CALC (1)
#define US_STATE_RECV (2)
#define US_STATE_FINISH_RECV (3)
#define US_STATE_GEST_START (4)
#define US_STATE_GEST_INGEST (5)
#define US_STATE_GEST_STOP (6)



void init_ultrasonic(void);
int get_distance(void);
int calc_distance(void);
void trig_us();
void trig_gest();
void PIN_INT0_IRQHandler();
void calc_gest();
void enable_gest();
void disable_gest();
void print_us_hist();
int get_us_state();
#endif /* ULTRASONIC_H_ */
