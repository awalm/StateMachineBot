/*
 * motor.h
 *
 *  Created on: Mar. 26, 2020
 *      Author: masta
 */

#ifndef ACTUATORS_H_
#define ACTUATORS_H_

//#define SERVO_PWM (21)
#define LEFT_DRIVE_PWM (11)
//#define RIGHT_DRIVE_PWM (19)
#define RIGHT_DRIVE_PWM (21)
#define RIGHT_DIR1 (8)
#define RIGHT_DIR2 (18)
#define LEFT_DIR1 (10)
#define LEFT_DIR2 (20)

//#define MAX_SPEED (1.0) //Speed scale from 0 to 1

#define SERVO_PWM_REG (0)
#define RIGHT_DRIVE_PWM_REG (1)
#define LEFT_DRIVE_PWM_REG (2)

#define CTIMER_FREQ (15000000)
#define PWM_PERIOD (50)
#define SERVO_RIGHT_DUTY (0.12)
#define SERVO_MIDDLE_DUTY (0.075)
#define SERVO_LEFT_DUTY (0.03)

void init_motors(void);
void drive(int left_speed, int right_speed);
void motor_ctrl(int dir1, int dir2, int speed, int motor_reg);
void move_servo(int angle);

#endif /* ACTUATORS_H_ */
