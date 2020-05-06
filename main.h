/*
 * main.h
 *
 *  Created on: Apr. 8, 2020
 *      Author: masta
 */

#ifndef MAIN_H_
#define MAIN_H_

#define MRT_TIMEOUT (3)
#define IDLE (0)
#define REMOTE_CONTROL (1)
#define LINE_TRACK (2)
#define RECORD (3)
#define REPLAY (4)
#define OBS_AVOID (5)
#define STOP (6)
#define REPLAY_MIRROR (7)
#define LINE_TRACK_LEFT (15)
#define LINE_TRACK_MIDDLE (13)
#define LINE_TRACK_RIGHT (16)
#define FORWARD_LEFT (100)
#define FORWARD_RIGHT (100)
#define RIGHT_RIGHT (0)
#define RIGHT_LEFT (100)
#define LEFT_RIGHT (100)
#define LEFT_LEFT (0)
#define BACKWARD_LEFT (-100)
#define BACKWARD_RIGHT (-100)

#define RECORDING_SIZE (16)
#define MRT_TIMEOUT_RELOAD_VAL (15000000) //Launch ~1.5 seconds

#define VIEW_RECORDING (8)
#define US_GEST (0)
#define US_MAN (1)
#define OBS_THRESHOLD (200) //Obstacle threshold in mm

void init_linetrack();
void init_timeout();
void line_track();
void obs_avoid();
void en_timeout();
void start();
void stop();
void change_mode(int new_mode);
void process_button(int button);
void lcd_drive(int left, int right);
void lcd_dir_char(int button);
void change_speed(int x);
void SysTick_Handler(void);

#endif /* MAIN_H_ */
