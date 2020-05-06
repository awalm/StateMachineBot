/*
 * motor.c
 *
 *  Created on: Mar. 26, 2020
 *      Author: masta
 */
#include <actuators.h>
#include <LPC804.h>
#include <pwm.h>
extern int speed_limit;
void init_motors(void) {
	//Configure direction pins of motor as output
	//Enable GPIO
	SYSCON->SYSAHBCLKCTRL0 |= (SYSCON_SYSAHBCLKCTRL0_GPIO0_MASK
			| SYSCON_SYSAHBCLKCTRL0_GPIO_INT_MASK);

//	//Reset GPIO
//	SYSCON -> PRESETCTRL0 &= ~(SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK | SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK);
//	SYSCON -> PRESETCTRL0 |= ~(SYSCON_PRESETCTRL0_GPIO0_RST_N_MASK | SYSCON_PRESETCTRL0_GPIOINT_RST_N_MASK);

	//Set the motor driver pins as outputs
	GPIO->DIRSET[0] = (1UL << RIGHT_DIR1) | (1UL << RIGHT_DIR2)
			| (1UL << LEFT_DIR1) | (1UL << LEFT_DIR2) | (1UL << RIGHT_DRIVE_PWM)
			| (1UL << LEFT_DRIVE_PWM);
	//Write 0 to all output pins so driver does not turn on

	GPIO->CLR[0] = (1UL << LEFT_DIR1) | (1UL << LEFT_DIR2) | (1UL << RIGHT_DIR1)
			| (1UL << RIGHT_DIR2);
	GPIO->CLR[0] = (1UL << LEFT_DRIVE_PWM) | (1UL << RIGHT_DRIVE_PWM);
}

void drive(int left_speed, int right_speed) {
	motor_ctrl(RIGHT_DIR1, RIGHT_DIR2, right_speed, RIGHT_DRIVE_PWM_REG);
	motor_ctrl(LEFT_DIR1, LEFT_DIR2, left_speed, LEFT_DRIVE_PWM_REG);
}

void motor_ctrl(int dir1, int dir2, int speed, int motor_reg) {
	//If speed is positive, set IN1 to high, IN2 to low to move forward, and vice versa
	if (speed > 0) {
		GPIO->SET[0] = (1UL << dir1);
		GPIO->CLR[0] = (1UL << dir2);
		//PWM
		pwm(motor_reg, ((speed / 100.00) * (speed_limit/100.0)));
	} else if (speed < 0) {
		GPIO->CLR[0] = (1UL << dir1);
		GPIO->SET[0] = (1UL << dir2);
		//PWM
		pwm(motor_reg, (-1.0 * (speed / 100.00) * (speed_limit/100.0)));
	} else {
		GPIO->CLR[0] = (1UL << dir1) | (1UL << dir2);
		pwm(motor_reg, 0);
	}
}
void move_servo(int angle) {
	//Check bounds
	if (angle < -90) {
		angle = -90;
	} else if (angle > 90) {
		angle = 90;
	}

	if (angle == -90) {
		pwm(SERVO_PWM_REG, SERVO_LEFT_DUTY);
	} else if (angle == 90) {
		pwm(SERVO_PWM_REG, SERVO_RIGHT_DUTY);
	} else if (angle == 0) {
		pwm(SERVO_PWM_REG, SERVO_MIDDLE_DUTY);
	} else {
		//y= 0.0005*​(x)+​0.03 is the trendline
		pwm(SERVO_PWM_REG, (0.0005*(angle +90)+0.03));
	}
}


