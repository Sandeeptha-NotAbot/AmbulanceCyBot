/*
 * movement.h
 *
 *  Created on: Sep 7, 2023
 *      Author: ieh
 */

#ifndef MOVEMENT_H_
#define MOVEMENT_H_
#include "open_interface.h"


int forward(oi_t*sensor_data, int centimeters);

void backward(oi_t*sensor_data, int centimeters);

double turn_clockwise(oi_t*sensor_data, double degrees);

void turn_counterclockwise(oi_t*sensor_data, double degrees);

//void smack_detetion(oi_t*sensor_data);

#endif /* MOVEMENT_H_ */
