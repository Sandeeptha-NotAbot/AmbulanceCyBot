/*
 * movement.h
 *
 *  Created on: Sep 7, 2023
 *      Author: ieh
 */

#include "movement.h"
#include "open_interface.h"
#include "uart_extra_help.h"

//Moves robot forward by specified centimeters
int forward(oi_t*sensor_data, int centimeters){
     double sum = 0;
     oi_setWheels(500, 500); // move forward; full speed
     while (abs(sum) < abs(centimeters*10)){
     oi_update(sensor_data);
     sum += sensor_data->distance;
     if(!(sensor_data->bumpLeft == 0  && sensor_data->bumpRight == 0)){
         break;
         }
     }
     oi_setWheels(0, 0); // stop
     return sum;

}

void backward(oi_t *sensor_data, int centimeters){
     double sum = 0;
     oi_setWheels(-500, -500); // move forward; full speed
     while (abs(sum) < abs(centimeters*10)){
         oi_update(sensor_data);
         sum += sensor_data->distance;
         }
     oi_setWheels(0, 0); // stop

}




//Turns robot clockwise by specified degrees
double turn_clockwise(oi_t *sensor_data, double degrees){
    double sum = 0;
    oi_update(sensor_data);
//    char send_data[10];
//    int i;
    oi_setWheels(-50, 50);
    while (fabs(sum) < fabs(degrees - 18)){ //corrections for cybot 3
        oi_update(sensor_data);
        sum += sensor_data->angle;
//        sprintf(send_data, "angle: %0.3f\n\r", sensor_data->angle);
//        for(i = 0; i < sizeof(send_data); i++){
//            uart_sendChar(send_data[i]);
//            }
        }
    oi_setWheels(0, 0); // stop
    return sum;
}

void turn_counterclockwise(oi_t *sensor_data, double degrees){
    double sum = 0;
    oi_update(sensor_data);
//    char send_data[10];
//    int i;
    oi_setWheels(50, -50);
    while (fabs(sum) < fabs(degrees - 18)){ //corrections for cybot 3
    oi_update(sensor_data);
    sum += sensor_data->angle;
//    sprintf(send_data, "angle: %0.3f\n\r", sensor_data->angle);
//    for(i = 0; i < sizeof(send_data); i++){
//        uart_sendChar(send_data[i]);
//        }
    }
    oi_setWheels(0, 0); // stop
}

void smack_detection(oi_t*sensor_data){

if(sensor_data->bumpLeft == 1  && sensor_data->bumpRight == 0){
    backward(sensor_data,15);
    turn_clockwise(sensor_data,90);
    forward(sensor_data,25);
    turn_counterclockwise(sensor_data,90);
}
else if(sensor_data->bumpLeft == 0  && sensor_data->bumpRight == 1){
    backward(sensor_data,15);
    turn_counterclockwise(sensor_data,90);
    forward(sensor_data,25);
      turn_clockwise(sensor_data,90);
}
else if(sensor_data->bumpLeft == 1  && sensor_data->bumpRight == 1){
    backward(sensor_data,15);
        turn_counterclockwise(sensor_data,90);
        forward(sensor_data,25);
          turn_clockwise(sensor_data,90);
}



}

