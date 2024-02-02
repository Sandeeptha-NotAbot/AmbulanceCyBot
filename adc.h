/*
 * adc.h
 *
 *  Created on: Oct 26, 2023
 *      Author: pnguyen2
 */

#ifndef ADC_H_
#define ADC_H_

#include <inc/tm4c123gh6pm.h>

void adc_init(void);
int adc_read(void);
void ir_cal(void);

#endif /* ADC_H_ */
