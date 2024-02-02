#include <Timer.h>
#include <stdio.h>
#include "movement.h"
#include <math.h>
#include <stdlib.h>
#include "adc.h"
#include "servo.h"
#include "uart_extra_help.h"
#include "ping.h"
#include "cliff_detection.h"

/**
 * main.c
 * 
 * Author: Phu N.
 */

//Stores the information for an object in object struct
typedef struct Object_Struct {
    float distance; //Centimeters
    int angle; //Degrees Start
    int middle; //Middle Angle
    int width; //Radial Degree
    int linearWidth; //cm
    int caliberatedIRMin; //cm
    int caliberatedIRFirst; //cm
    int caliberatedIRLast; //cm
} Object_t;

//Function declarations
void printStringToPutty(char str[]);
float min(float a, float b);
float max(float a, float b);
int rawToCm(int raw);
int avgIR(void);
int getDistAtAngle(int angle);
void switchMan(int* switchMode);
void play_song();

int main(void)
{
    //Variables
    int const MAX_DISTANCE = 50;
    int const MAX_MARGIN = 10;

    int angle = 0;
    int getByte = 0;
    char sensorData[100];
    char outcomeData[100];
    int itemInView = 0;
    int switchManualAuto = 1; // 1: Manual, 0: Auto
    char command[100];
    int currentAngle = 0;
    int direction = 1;
    int directionOLD = 0;
    int i;

    //Flags
    int edge_flag;
    int edge = 0;
    int bump_flag = 0;
    int old_edge_flag = 0;
    int bumped = 0;
    int stopFlag;

    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

    //Initiate
    uart_init(115200);
    uart_interrupt_init();
    timer_init();
    adc_init();
    ping_init();
    servo_init();
    lcd_init();
    //servo_cal();
    servo_set_edge(36450, 6566);
    //ir_cal();

    //Main while loop
    while (1) {
        //UART interrupt reciever, getByte is either the data or null byte
        if (flag == 1) {
            flag = 0;
            getByte = uart_data;
        }
        else {
            getByte = 0;
        }

        //Only update open interface iangle is 180, 90, or 0 degrees on the scanner
        if (currentAngle % 90 == 0) {
            oi_update(sensor_data);
        }

        //SENDING DATA

        //Checks bump and edge
        edge_flag = edge_check(sensor_data);
        bump_flag = bump_check(sensor_data);

        //Sends a single command if bump is hit
        if (bumped == 0 && bump_flag != 0) {
            bumped = 1;
            //Sends different character based on which bump was hit
            if (bump_flag == 0b01) {
                timer_waitMillis(100);
                uart_sendChar('l');
            }
            else if (bump_flag == 0b10) {
                timer_waitMillis(100);
                uart_sendChar('r');
            }
            else if (bump_flag == 0b11) {
                timer_waitMillis(100);
                uart_sendChar('f');
            }
        }
        //Send single command if bump isn't being hit
        else if (bumped == 1 && bump_flag == 0) {
            bumped = 0;
            timer_waitMillis(100);
            uart_sendChar('b');
        }

        //Sends a single command if edge is detected
        if (edge_flag != 0 && edge != edge_flag) {
            char tempData[2];
            edge = edge_flag;
            lcd_puts("e");
            timer_waitMillis(100);
            sprintf(tempData, "e%c", (char)edge_flag); //Send which edge is being triggered
            printStringToPutty(tempData);
        }
        //Send single command if edge isn't being scanned
        else if (edge > 0 && edge_flag == 0) {
            edge = 0;
            lcd_puts("k");
            timer_waitMillis(100);
            uart_sendChar('k');
        }

        //Changes direction to zero if it isn't (stops the scanning) or changes direction back to what it was before it was stopped
        if (getByte == 'p') {
            if (direction != 0) {
                directionOLD = direction;
                direction = 0;
            }
            else
                direction = directionOLD;
        }

        //RECIEVING DATA

        //Pings where the scanner is pointed and sends data
        if (getByte == 'o') {
            char tempData[10];
            sprintf(tempData, "o%d", pingToCM(ping_read()));
            printStringToPutty(tempData);
        }

        //Switches to auto or man modes
        if (getByte == 'n') {
            switchMan(&switchManualAuto);
        }

        //Moves forward if 'w' is recieved, only works if both edge, bump, and ir sensors don't detect an object
        if (getByte == 'w' && stopFlag == 0 && edge == 0 && bumped == 0) {
            oi_setWheels(50, 50);
        }

        //Goes backwards until told to stop
        if (getByte == 's') {
            oi_setWheels(-50,-50);
        }

        //Stops all wheels
        if (getByte == ' ') {
            oi_setWheels(0,0);
        }

        //Rotate left until stop command
        if (getByte == 'a') {
            oi_setWheels(50, -50);
        }

        //Rotate right until stop command
        if (getByte == 'd') {
            oi_setWheels(-50, 50);
        }

        //Plays song
        if (getByte == 'c') {
            play_song();
        }

        //Gets IR scan and converts it to cm into rawCM, if larger than MAX_DISTANCE use MAX_DISTANCE
        int rawCM = min(rawToCm(adc_read()), MAX_DISTANCE);

        //Stop wheels if an item is scanned within 15 cm of Cybot and it isn't already stopped
        if (rawCM < 15 && stopFlag == 0) {
            stopFlag = 1;

            oi_setWheels(0,0);
        }
        //Reset stop flag if object has been avoided (rotate or back up)
        else if (rawCM > 18) {
            stopFlag = 0;
        }

        //Only scan if there isn't an object right infront of the cybot
        if (stopFlag == 0) {
            //Scan only scans once until the next while loop, this allows other action to be interwoven between scans allowing for multi tasking
            //Switch direction if 180 is reached
            if (currentAngle >= 180) {
                direction = -1;
                timer_waitMillis(10);
            }
            //Switch direction if 0 is reached
            else if (currentAngle <= 0) {
                direction = 1;
                timer_waitMillis(10);
            }

            //Increment global angle variable depending on which direction the scanner is moving
            currentAngle += direction;

            //Send scan data to the Cybot
            sprintf(sensorData, "\n\r%7d %7d\n\r", currentAngle, rawCM);
            printStringToPutty(sensorData);

            //Move servo to the next location
            servo_move(currentAngle);
        }
        // Only send a '\n' if the first byte of the command is not a '\n',
        // to avoid sending back-to-back '\n' to the client
        if(command[0] != '\n')
        {
            uart_sendChar('\n');
        }
    }

    oi_free(sensor_data);
}

//Prints any string given to putty
void printStringToPutty(char str[]) {
    int i;

    for (i = 0; i < strlen(str); i++) {
        uart_sendChar(str[i]);
    }
}

//Returns min value
float min(float a, float b) {
    return (a < b) ? a : b;
}

//Returns max value
float max(float a, float b) {
    return (a > b) ? a : b;
}

//Converts raw IR data to centimeters
int rawToCm(int raw) {
    return (double)3000000 * pow((double)raw, -1.575);
}

//Gets the average IR of 3 scans
int avgIR() {
    int i = 0;
    int totalIR = 0;

    for (i = 0; i < 3; i++) {
        totalIR += adc_read();
    }

    return totalIR / 3;
}

//Gets average ping distance at an angle
int getDistAtAngle(int angle) {
    int totalDist = 0;
    int i = 0;
    servo_move(angle);

    for (i = 0; i < 5; i++) {
        totalDist += pingToCM(ping_read());
        timer_waitMillis(100);
    }

    return totalDist / 5;
}

//Switches to manual/auto mode
void switchMan(int* switchMode) {
    if (*switchMode == 1) {
         *switchMode = 0;
         printStringToPutty("Switched to Auto");
     }
     else {
         *switchMode = 1;
         printStringToPutty("Switched to Manual");
     }
}

void play_song() {
    uint8_t goodLandingSpotNumNotes = 5;
    uint8_t goodLandingSpotNotes[5] = {95,88,95,88,95};
    uint8_t goodLandingSpotDurations[5] = {64,64,64,64,64};

    oi_loadSong(0, goodLandingSpotNumNotes, goodLandingSpotNotes, goodLandingSpotDurations);
    oi_play_song(0);
}
