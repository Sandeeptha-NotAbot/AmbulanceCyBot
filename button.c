/*
 * button.c
 *
 *  Created on: Jul 18, 2016
 *      Author: Eric Middleton, Zhao Zhang, Chad Nelson, & Zachary Glanz.
 *
 *  @edit: Lindsey Sleeth and Sam Stifter on 02/04/2019
 *  @edit: Phillip Jone 05/30/2019: Mearged Spring 2019 version with Fall 2018
 */
 


//The buttons are on PORTE 3:0
// GPIO_PORTE_DATA_R -- Name of the memory mapped register for GPIO Port E, 
// which is connected to the push buttons
#include "button.h"
#include <stdio.h>

// Global varibles
volatile int button_event;
volatile int button_num;

/**
 * Initialize PORTE and configure bits 0-3 to be used as inputs for the buttons.
 */
void button_init() {
    static uint8_t initialized = 0;

    //Check if already initialized
    if(initialized){
        return;
    }

    // Reading: To initialize and configure GPIO PORTE, visit pg. 656 in the
    // Tiva datasheet.

    // Follow steps in 10.3 for initialization and configuration. Some steps
    // have been outlined below.

    // Ignore all other steps in initialization and configuration that are not
    // listed below. You will learn more about additional steps in a later lab.

    // 1) Turn on PORTE system clock, do not modify other clock enables
    SYSCTL_RCGCGPIO_R |= 0b00010000;

    // 2) Set the buttons as inputs, do not modify other PORTE wires
    GPIO_PORTE_DIR_R &= 0b11110000;

    // 3) Enable digital functionality for button inputs,
    //    do not modify other PORTE enables
    GPIO_PORTE_DEN_R |= 0b00001111;


    initialized = 1;
}



/**
 * Initialize and configure PORTE interupts
 */
void init_button_interrupts() {

    // In order to configure GPIO ports to detect interrupts, you will need to visit pg. 656 in the Tiva datasheet.
    // Notice that you already followed some steps in 10.3 for initialization and configuration of the GPIO ports in the function button_init().
    // Additional steps for setting up the GPIO port to detect interrupts have been outlined below.
    // TODO: Complete code below

    // 1) Mask the bits for pins 0-3
    GPIO_PORTE_IM_R &= 0b11110000;

    // 2) Set pins 0-3 to use edge sensing
    GPIO_PORTE_IS_R &= 0b11110000;

    // 3) Set pins 0-3 to use both edges. We want to update the LCD
    //    when a button is pressed, and when the button is released.
    GPIO_PORTE_IBE_R |= 0b00001111;

    // 4) Clear the interrupts
    GPIO_PORTE_ICR_R |= 0b00001111;

    // 5) Unmask the bits for pins 0-3
    GPIO_PORTE_IM_R |= 0b00001111;

    // TODO: Complete code below
    // 6) Enable GPIO port E interrupt
    NVIC_EN0_R |= 0b00010000;

    // Bind the interrupt to the handler.
    IntRegister(INT_GPIOE, gpioe_handler);
}

/**
 * Interrupt handler -- executes when a GPIO PortE hardware event occurs (i.e., for this lab a button is pressed)
 */
void gpioe_handler() {
    if (GPIO_PORTE_RIS_R & 0x0F) {
        button_num = button_getButton();
        button_event = 1;

        // Clear interrupt status register
        GPIO_PORTE_ICR_R |= 0b00001111;
    }
}



/**
 * Returns the position of the rightmost button being pushed.
 * @return the position of the rightmost button being pushed. 4 is the rightmost button, 1 is the leftmost button.  0 indicates no button being pressed
 */
uint8_t button_getButton() {

    //
    // DELETE ME - How bitmasking works
    // ----------------------------------------
    // In embedded programming, often we only care about one or a few bits in a piece of
    // data.  There are several bitwise operators that we can apply to data in order
    // to "mask" the bits that we don't care about.
    //
    //  | = bitwise OR      & = bitwise AND     ^ = bitwise XOR     ~ = bitwise NOT
    //        << x = shift left by x bits        >> x = shift right by x bits
    //
    // Let's say we want to know if push button 3 (S3) of GPIO_PORTE_DATA_R is
    // pushed.  Since push buttons are high (1) initially, and low (0) if pushed, PORTE should
    // look like:
    // GPIO_PORTE_DATA_R => 0b???? ?0?? if S3 is pushed
    // GPIO_PORTE_DATA_R => 0b???? ?1?? if S3 is not pushed
    //
    // This is not useful: There are 128 different 8 bit numbers that have the 3rd bit high or low.
    // We can make it more clear if we mask the other 7 bits:
    //
    // Bitwise AND:
    // (GPIO_PORTE_DATA_R & 0b0000 0100) => 0b0000 0000 if S3 is pushed
    // (GPIO_PORTE_DATA_R & 0b0000 0100) => 0b0000 0100 if S3 is not pushed
    //
    // Bitwise OR:
    // (GPIO_PORTE_DATA_R | 0b1111 1011) => 0b1111 1011 if S3 is pushed
    // (GPIO_PORTE_DATA_R | 0b1111 1011) => 0b1111 1111 if S3 is not pushed
    //
    // Other techniques (Shifting and bitwise AND)
    // ((GPIO_PORTE_DATA_R >> 2) & 1) => 0 if S3 is pushed
    // ((GPIO_PORTE_DATA_R >> 2) & 1) => 1 if S3 is not pushed

    // TODO: Write code below -- Return the left must button position pressed

    int i = 0;

    //Index of left most bit => right most button.
    int rightMost = -1;

    //Iterate through first 4 bits of PortE and shifts it right by 1 each time. Records the left most bit and returns that.
    for (i = 0; i < 4; i++) {
        if (((GPIO_PORTE_DATA_R >> i) & 0x1) == 0)
            rightMost = i;
    }

    //Add 1 to fit specifications
    return rightMost + 1;
}




