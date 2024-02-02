/*
 * cliff_detection.c
 *
 *  Created on: Nov 30, 2023
 *      Author: ieh
 */


#include "cliff_detection.h"

int edge_check(oi_t *self) {

   int cliff_flag = 0b0000;

   if(self->cliffLeftSignal > 2400 || self->cliffLeftSignal < 500){
        cliff_flag = 0b1000;
   }
   if(self->cliffFrontLeftSignal > 2600 || self->cliffFrontLeftSignal < 500){
       cliff_flag += 0b0100;
   }
   if(self->cliffFrontRightSignal > 2400 || self->cliffFrontRightSignal < 500){
       cliff_flag += 0b0010;
   }
   if(self->cliffRightSignal > 2600 || self->cliffRightSignal < 500){
        cliff_flag = 0b0001;
   }

   return cliff_flag;
}


int bump_check(oi_t *self){
    int bump_flag = 0b00;

    if(self->bumpRight > 0){
        bump_flag += 0b10;
    }
    if(self->bumpLeft > 0){
       bump_flag += 0b01;
    }

   return bump_flag;
}


//void edge_correction(int flag){
//    if(value == 0b0110 || value == 0b1001 || 0b1111){
//        //turn 180
//    }
//    if(value == 0b1000 || value == 0b0100){
//        //turn clockwise 90
//    }
//    if(value == 0b0001 || value == 0b0010){
//        //turn counter clockwise by 90
//    }
//    if()
//}

//CYBOT 12

//left = 1834
//Fleft == 1681
//fright ==645
//right 2448

//CYBOT 7
//left = 2000
//Fleft == 2600
//fright == 2300
//right 2448

//CYBOT  6
//FOR white
//left = 2802
//Fleft == 2802
//fright == 2792
//right 2718
