/*
 * adc.c
 *
 *  Created on: Oct 26, 2023
 *      Author: pnguyen2
 */
#include "lcd.h"
#include "button.h"
#include "Timer.h"
#include "uart_extra_help.h"
#include "servo.h"

extern volatile int button_event;
extern volatile int button_num;

void adc_init(void) {
    SYSCTL_RCGCADC_R |= 0b1;
    SYSCTL_RCGCGPIO_R |= 0b10;
    GPIO_PORTB_AFSEL_R |= 0b10000;
    GPIO_PORTB_DIR_R &= 0b01111;
    GPIO_PORTB_DEN_R &= 0b01111;
    GPIO_PORTB_AMSEL_R |= 0b10000;

    ADC0_ACTSS_R &= 0xFFFE;
    ADC0_EMUX_R |= 0x0;
    GPIO_PORTB_ADCCTL_R &= 0x00;

    ADC0_SSMUX0_R |= 0xA;
    ADC0_SSMUX0_R &= 0xA;

    ADC0_SSCTL0_R |= 0x6;
    ADC0_SSCTL0_R &= 0x6;

    ADC0_ACTSS_R |= 0x1;
}

int adc_read(void) {
    ADC0_PSSI_R |= 0b1;

    return ADC0_SSFIFO0_R;
}

void ir_cal(void) {
    lcd_init();
    button_init();
    init_button_interrupts();

    servo_move(90);

    int i;
    int j;
    char value[100];
    char out[100];

    lcd_puts("Press 2 to collect data, press 1 to move onto next distance");

    while (1) {
        if (button_event == 1 && button_num == 1) {
            break;
        }
    }

    for (i = 0; i < 5; i++) {
        sprintf(out, "Move to %d cm", (i + 1) * 10);
        lcd_clear();
        lcd_home();
        lcd_puts(out);

        while (1) {
            if (button_event == 1 && button_num == 2) {
                break;
            }
        }

        int val[3];

        adc_read();
        timer_waitMillis(500);

        for (j = 0; j < 3; j++) {
            val[j] = adc_read();
            timer_waitMillis(500);
        }
        sprintf(value, "Values: %d, %d, %d", val[0], val[1], val[2]);
        lcd_clear();
        lcd_home();
        lcd_puts(value);

        while (1) {
            if (button_event == 1 && button_num == 1) {
                break;
            }
        }
    }
}
