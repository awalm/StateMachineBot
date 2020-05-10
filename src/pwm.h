/*
 * pwm.h
 *
 *  Created on: Mar. 25, 2020
 *      Author: masta
 */

#ifndef PWM_H_
#define PWM_H_

int init_pwm(void);
int pwm(int pwm_reg, double duty_cycle);

#endif /* PWM_H_ */
