/*
*
*   uart_extra_help.c
* Description: This is file is meant for those that would like a little
*              extra help with formatting their code, and followig the Datasheet.
*/

#include <math.h>
#include "lcd.h"
#include "timer.h"
#define REPLACE_ME 0x00

// Adding global volatile varibles for communcating between
// your Interupt Service Routine, and your non-interupt code.

volatile  char uart_data;  // Your UART interupt code can place read data here
volatile  char flag;       // Your UART interupt can update this flag
                           // to indicate that it has placed new data
                           // in uart_data

void uart_init(int baud)
{
    SYSCTL_RCGCGPIO_R |= 0b00000010;      // enable clock GPIOB (page 340)
    SYSCTL_RCGCUART_R |= 0b00000010;      // enable clock UART1 (page 344)
    GPIO_PORTB_AFSEL_R |= 0b00000011;      // sets PB0 and PB1 as peripherals (page 671)
    GPIO_PORTB_PCTL_R |= 0x00000011;       // pmc0 and pmc1       (page 688)  also refer to page 650
    GPIO_PORTB_PCTL_R &= 0xFFFFFF11;
    GPIO_PORTB_DEN_R  |= 0b00000011;        // enables pb0 and pb1
    GPIO_PORTB_DIR_R  &= 0b11111110;        // sets pb0 as input
    GPIO_PORTB_DIR_R  |= 0b00000010;        // sets pb1 as output

    //compute baud values [UART clock= 16 MHz] 
    double fbrd;
    int ibrd;

    double brd = 16000000.0 / (16.0 * baud); // page 903
    ibrd = floor(brd);
    fbrd = brd - ibrd;

    UART1_CTL_R &= 0xFFFE;      // disable UART1 (page 918)
    UART1_IBRD_R = ibrd;        // write integer portion of BRD to IBRD
    UART1_FBRD_R = (int)(fbrd * 64 + 0.5);   // write fractional portion of BRD to FBRD
    UART1_LCRH_R &= 0b11111101;        // write serial communication parameters (page 916) * 8bit and no parity
    UART1_LCRH_R |= 0b01100000;
    UART1_CC_R   &= 0b11110000;          // use system clock as clock source (page 939)
    UART1_CTL_R |= 0x301;        // enable UART1
}

void uart_sendChar(char data)
{
    while (UART1_FR_R & 0b00100000) {

    }

    UART1_DR_R = data;
}

char uart_receive(void)
{
    char data = 0;

    while(UART1_FR_R & 0b00010000) {

    }

    data = (char)(UART1_DR_R & 0xFF);

    return data;
}

void uart_sendStr(const char *data)
{

}

// _PART3

void uart_interrupt_handler() {
// STEP1: Check the Masked Interrup Status

//STEP2:  Copy the data 

//STEP3:  Clear the interrup   
    if (UART1_MIS_R & 0b00010000) {
        uart_data = (char)(UART1_DR_R & 0xFF);
        flag = 1;

        // Clear interrupt status register
        UART1_ICR_R |= 0b00010000;
    }
}


void uart_interrupt_init()
{
    // Enable interrupts for receiving bytes through UART1
    UART1_IM_R |= 0b00010000; //enable interrupt on receive - page 924

    // Find the NVIC enable register and bit responsible for UART1 in table 2-9
    // Note: NVIC register descriptions are found in chapter 3.4
    NVIC_EN0_R |= 0b01000000; //enable uart1 interrupts - page 104

    // Find the vector number of UART1 in table 2-9 ! UART1 is 22 from vector number page 104
    IntRegister(INT_UART1, uart_interrupt_handler); //give the microcontroller the address of our interrupt handler - page 104 22 is the vector number
}
