#ifndef SW_FCNS_H
#define SW_FCNS_H
#include "adc.h"

extern volatile unsigned int brightness;
volatile unsigned int x;
volatile unsigned int decreasing_brightness = 0;
float multiplier = 0.5f;
extern int normal;
extern int pls_freeze;
#define IN_RANGE(n, min, max) ((n) >= (min) && (n) < (max))
#define INIT_TOP 468 // Let x be the PER value, $100 Hz=\frac{48e6}{1024(x+1)}$

/*
0: 7a1fce -> R : 122, G : 31, B : 206
3: e57c16 -> R : 229, G : 124, B : 22
1: 4fe516 -> R : 79, G : 229, B : 22
2: 16dde5 -> R : 22, G : 221, B : 229
6: 10e96e -> R : 16, G : 233, B : 110
 */
uint16_t RGB_to_CC(uint8_t x);
int direction = 1;
int freeze = 0;
int i;
int j;
int k;
int z;
int came_from_freeze = 0;

int colors[5][3] = {
    {122, 31, 206},
    {229, 124, 22},
    {79, 229, 22},
    {22, 221, 229},
    {16, 233, 110},
};

int read_count() {
    // Allow read access of COUNT register
    // Return back the counter value
    TC0_REGS -> COUNT16.TC_CTRLBSET |= (0x4 << 5);
    return TC0_REGS -> COUNT16.TC_COUNT; // 39.8.13

}

void TC0_Wait(void) {
    // Clear the interrupt flag for match compare 0
    TC0_REGS->COUNT16.TC_INTFLAG = (1 << 4);

    // Start the timer by resetting the counter
    TC0_REGS->COUNT16.TC_CTRLBSET = (1 << 0); // Command: Restart
    while (TC0_REGS->COUNT16.TC_SYNCBUSY & (1 << 0)); // Wait for sync

    // Wait for the match compare flag to be set
    while (!(TC0_REGS->COUNT16.TC_INTFLAG & (1 << 4))) {
        // Do nothing, just wait for the timer to reach the compare value
    }

    // Clear the interrupt flag again after the wait
    TC0_REGS->COUNT16.TC_INTFLAG = (1 << 4);
}

void Cycle_RGB(int normal) {
    /*
     * Cycles through RGB colors based on direction and multiplier, respecting freeze state.
     * @param multiplier: the multiplier for adjusting brightness.
     * @param normal: the direction (1 for forward, 0 for reverse).
     */
    // Initial Freeze
    if (pls_freeze == 1) {
        freeze = 1;
        pls_freeze = 0;
        i = 0;
    }

    if (freeze == 1) {
        TCC3_REGS->TCC_CC[1] = RGB_to_CC(multiplier * colors[i][0]);
        TCC3_REGS->TCC_CC[0] = RGB_to_CC(multiplier * colors[i][1]);
        TCC3_REGS->TCC_CC[3] = RGB_to_CC(multiplier * colors[i][2]);
    } else {
        if (freeze == 1) {
            return;
        }
        // If normal direction
        if (normal == 1 && i >= 0 && i <= 4) {
            // Check for freeze
            if (freeze == 1) {
                return;
            }
            TC0_Wait();
            // Check for freeze
            if (freeze == 1) {
                return;
            }
            i++;
            if (i == 5) {
                i = 0;
            } // If reverse direction
        } else if (normal == 0 && i >= 0 && i <= 4) {
            // Check freeze
            if (freeze == 1) {
                return;
            }
            TC0_Wait();
            i--;
            if (i == -1) {
                i = 4;
            }

        }

        TCC3_REGS->TCC_CC[1] = RGB_to_CC(multiplier * colors[i][0]);
        TCC3_REGS->TCC_CC[0] = RGB_to_CC(multiplier * colors[i][1]);
        TCC3_REGS->TCC_CC[3] = RGB_to_CC(multiplier * colors[i][2]);
        // Check freeze
        if (freeze == 1) {
            return;
        }
    }

}

void Adjust_Brightness(uint16_t adc_value) {
    /**
     * A simple function that overwrites the global variable multiplier depending on the adc_value.
     * Since it is 10 bits, we want the multiplier to be 1 when ADC value is 0 (full clockwise) and vice versa.
     * @param adc_value: the adc value from the potentiometer reading.
     */
    multiplier = 1 - (adc_value / 1028.0f);
    TCC3_REGS->TCC_CC[1] = RGB_to_CC(multiplier * colors[i][0]);
    TCC3_REGS->TCC_CC[0] = RGB_to_CC(multiplier * colors[i][1]);
    TCC3_REGS->TCC_CC[3] = RGB_to_CC(multiplier * colors[i][2]);
}

void Adjust_Period_and_Direction(uint16_t adc_value) {
    /* The function that adjusts the period per color and the direction.
     * ADC Value: {%} * 2^10
     * 0-20%: 0-205.8
     * 20%-40%: 205.8 - 409.6
     * 40%-60%: 614.4
     * 60%-80%: 819.2
     * 80%-100%: 1024
     * To avoid floating point errors, rounded off the nearest whole number.
     * @param adc_value: from the potentiometer input.
     */
    if (IN_RANGE(adc_value, 0, 206)) {
        normal = 1;
        while (TC0_REGS->COUNT16.TC_SYNCBUSY & (1 << 0));
        TC0_REGS -> COUNT16.TC_CC[0] = (0x32C8);
        while (TC0_REGS->COUNT16.TC_SYNCBUSY & (1 << 0));
        freeze = 0;
    } else if (IN_RANGE(adc_value, 206, 410)) {
        normal = 1;
        while (TC0_REGS->COUNT16.TC_SYNCBUSY & (1 << 0));
        TC0_REGS -> COUNT16.TC_CC[0] = (0x32C8) * 2;
        while (TC0_REGS->COUNT16.TC_SYNCBUSY & (1 << 0));
        freeze = 0;
    } else if (IN_RANGE(adc_value, 410, 614)) {
        freeze = 1;
        came_from_freeze = 1;
    } else if (IN_RANGE(adc_value, 614, 819)) {
        normal = 0;
        while (TC0_REGS->COUNT16.TC_SYNCBUSY & (1 << 0));
        TC0_REGS -> COUNT16.TC_CC[0] = (0x32C8) * 2;
        while (TC0_REGS->COUNT16.TC_SYNCBUSY & (1 << 0));
        freeze = 0;
    } else if (IN_RANGE(adc_value, 819, 1024)) {
        normal = 0;
        while (TC0_REGS->COUNT16.TC_SYNCBUSY & (1 << 0));
        TC0_REGS -> COUNT16.TC_CC[0] = (0x32C8);
        while (TC0_REGS->COUNT16.TC_SYNCBUSY & (1 << 0));
        freeze = 0;
    }
}

uint16_t RGB_to_CC(uint8_t x) {
    /**
     * An RGB to CC converter so I don't have to manually calculate converting from RGB to CC.
     * @param x: Either R, G, or B.
     * @return: Appropriate CC PER value.
     */
    return (uint16_t) (INIT_TOP * (1.0f - (float) x / 255.0f));
}
#endif
