/*
 * servo.c
 *
 *  Created on: Nov 9, 2023
 *      Author: pnguyen2
 */


#include "servo.h"
#include "lcd.h"
#include "button.h"

extern volatile int button_event;
extern volatile int button_num;

int left_degree;
int right_degree;
int one_degree;

void servo_init() {
    SYSCTL_RCGCGPIO_R |= 0b10; //Enable Port B
    SYSCTL_RCGCTIMER_R |= 0b10; //Enable Timer 1

    GPIO_PORTB_DEN_R |= 0b00100000; //Enable Pin 3
    GPIO_PORTB_DIR_R |= 0b00100000; //Set Pin 3 direction
    GPIO_PORTB_AFSEL_R |= 0b00100000; //Enable Alt Func
    GPIO_PORTB_PCTL_R |= 0x00700000; //Select Timer 1B
    GPIO_PORTB_PCTL_R &= 0xFF7FFFFF; //Select Timer 1B

    TIMER1_CTL_R &= 0xFEFF; //Disable Timer1B
    TIMER1_CFG_R = 0x4; //16 bit timer

    TIMER1_TBMR_R |= 0x00A;//Enable periodic mode,edgecount,pwm
    TIMER1_TBMR_R &= 0xFFA;

    TIMER1_CTL_R |= 0x4C00;

    TIMER1_TBILR_R = 0xE200; //Set Pulse to 20ms
    TIMER1_TBPR_R = 0x04;

    TIMER1_TBMATCHR_R = 0x3E80; //Set Match to 1ms
    one_degree = 90;

    TIMER1_CTL_R |= 0x0100; //Enable B
}

void servo_set_edge(int left, int right) {
    left_degree = left;
    right_degree = right;

    TIMER1_CTL_R &= 0xFEFF;

    TIMER1_TBMATCHR_R = right; //Set Match to 1ms
    one_degree = (left - right) / 180;

    TIMER1_CTL_R |= 0x0100; //Enable B
}

void servo_cal(void) {
    lcd_init();
    servo_init();
    button_init();
    init_button_interrupts();

    char info[21];
    int direction = 1;

    lcd_gotoLine(2);
    lcd_puts("[1]1 deg : [2] 5 deg");
    lcd_gotoLine(3);
    lcd_puts("[3]clockwise or ccw");
    lcd_gotoLine(4);
    lcd_puts("[4]0 deg or 180 deg");

    while(1)
    {
        //Reset cursor to 0,0
        lcd_home();
        sprintf(info,"Dir:%2d     Val:%5d", direction, TIMER1_TBMATCHR_R);
        lcd_puts(info);

        if (button_event == 0) {
            continue;
        }

        if (button_num == 1)
            TIMER1_TBMATCHR_R += 89 * direction; //Increment
        else if (button_num == 2)
            TIMER1_TBMATCHR_R += 445 * direction; //Increment
        else if (button_num == 3)
            direction = direction * -1;
        else if (button_num == 4) {
            if (direction == 1)
                TIMER1_TBMATCHR_R = 16000;
            else
                TIMER1_TBMATCHR_R = 32000;
        }

        button_event = 0;
    }
}

void servo_move(int degrees) {
    TIMER1_TBMATCHR_R = right_degree + degrees * one_degree;

    timer_waitMillis(3.5);
}
