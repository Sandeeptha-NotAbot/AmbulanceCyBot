/*
 * ping.c
 *
 *  Created on: Nov 2, 2023
 *      Author: pnguyen2
 */

#include "ping.h"

volatile enum {LOW, HIGH, DONE} state; //Set by ISR
volatile unsigned int rising_time; //Pulse start time
volatile unsigned int falling_time; //Pulse start time
volatile unsigned int overflow;

void ping_init(void) {
    rising_time = 0;
    falling_time = 0;
    state = DONE;

    SYSCTL_RCGCGPIO_R |= 0b10; //Enable Port B
    SYSCTL_RCGCTIMER_R |= 0b1000; //Enable Timer 3

    GPIO_PORTB_DEN_R |= 0b1000; //Enable Pin 3
    GPIO_PORTB_DIR_R |= 0b1000; //Set Pin 3 direction
    GPIO_PORTB_AFSEL_R |= 0b1000; //Enable Alternative Function for pin 3
    GPIO_PORTB_PCTL_R |= 0x00007000; //Set pin 3 to use timer alternative function
    GPIO_PORTB_PCTL_R &= 0xFFFF7FFF; //Set pin 3 to use timer alternative function

    TIMER3_CTL_R &= 0xFEFF; //Disable Timer B

    TIMER3_CTL_R |= 0x0C00; //Enable both edge

    TIMER3_CFG_R = 0x4; //16 bit timer

    TIMER3_TBMR_R |= 0x07; //Enable capture mode and edge time mode
    TIMER3_TBMR_R &= 0xEF; //Countdown

    TIMER3_TBILR_R |= 0xFFFFF; //Set Upperbound

    NVIC_EN1_R |= 0x10; //Enable Interrupts for device

    IntRegister(INT_TIMER3B, TIMER3B_Handler);
    IntMasterEnable();

    TIMER3_CTL_R |= 0x0100; //Enable Timer B
}

void TIMER3B_Handler(void) {
    TIMER3_ICR_R |= 0x400; //Clear Interrupt

    if(state == LOW) {
        rising_time = TIMER3_TBR_R;
        state = HIGH;
    }
    else if (state == HIGH) {
        falling_time = TIMER3_TBR_R;
        state = DONE;
    }
}

int ping_read(void) {
    int pulseWidthRaw;
    int pulseWidthCM;

    state = LOW;
    send_pulse();

    while(state != DONE) { };

    if (rising_time < falling_time) {
        pulseWidthRaw = (TIMER3_TBILR_R - falling_time) + rising_time;
        overflow++;

//        char output[100];
//        sprintf(output, "Overflow: %d\n\r", overflow);
//        puts(output);
    }
    else
        pulseWidthRaw = rising_time - falling_time;

    rising_time = 0;
    falling_time = 0;

    return pulseWidthRaw;
}

int pingToCM(int pulseWidth) {
    return pulseWidth / 2.0 * (34000.0 / 16000000.0);
}

void send_pulse(void) {
    TIMER3_IMR_R &= ~0x0400; //Mask Timer3B Int

    GPIO_PORTB_AFSEL_R &= 0b11110111; //Disable Alternative Function for pin 3

    GPIO_PORTB_DIR_R |= 0b00001000; //Set Pin 3 direction to output
    GPIO_PORTB_DATA_R |= 0b00001000; //Set High
    timer_waitMicros(5);
    GPIO_PORTB_DATA_R &= 0b11110111; //Set Low
    GPIO_PORTB_DIR_R &= 0b11110111; //Set Pin 3 direction to input

    GPIO_PORTB_AFSEL_R |= 0b1000; //Enable Alternative Function for pin 3

    TIMER3_ICR_R |= 0x0400; //Clear Interrupt
    TIMER3_IMR_R |= 0x0400; //Unmask Timer3B Int
}

