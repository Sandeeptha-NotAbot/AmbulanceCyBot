/*
 * ping.h
 *
 *  Created on: Nov 2, 2023
 *      Author: pnguyen2
 */

#ifndef PING_H_
#define PING_H_

#include <inc/tm4c123gh6pm.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "driverlib/interrupt.h"
#include "Timer.h"

void ping_init(void);
int ping_read(void);
int pingToCM(int pulseWidth);
void TIMER3B_Handler(void);
void send_pulse(void);

#endif /* PING_H_ */
