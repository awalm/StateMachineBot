/*
 * recording.h
 *
 *  Created on: Apr. 8, 2020
 *      Author: masta
 */

#ifndef RECORDING_H_
#define RECORDING_H_

#define DIR_STOP (0)
#define DIR_FORWARD (1)
#define DIR_RIGHT (2)
#define DIR_LEFT (-2)
#define DIR_BACKWARD (-1)

//Function declarations
void add_record_cmd(int button);
void lcd_dir_str(int button);
void lcd_dir_str_drv(int button);
void replay_record(int dir);
void drive_dir(int dir);
void disp_only_record() ;
void disp_record();
void delete_record();
int get_cmd_idx();

#endif /* RECORDING_H_ */
