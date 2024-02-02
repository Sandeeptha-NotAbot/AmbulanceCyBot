/*
 * servo.h
 *
 *  Created on: Nov 9, 2023
 *      Author: pnguyen2
 */

#ifndef SERVO_H_
#define SERVO_H_

#include <inc/tm4c123gh6pm.h>
#include <stdbool.h>
#include <stdint.h>
#include "driverlib/interrupt.h"

//Run servo cal first
void servo_init(void);

void servo_set_edge(int left, int right);

//Run this get values for 0 degrees and 180 degrees then use servo_set_edge(180 degree value,0 180 degree value)
void servo_cal(void);

void servo_move(int degrees);

#endif /* SERVO_H_ */
