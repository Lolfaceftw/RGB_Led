#ifndef SW_FCNS_H
#define SW_FCNS_H
#include "adc.h"

extern volatile unsigned int brightness;
volatile unsigned int x;
volatile unsigned int decreasing_brightness = 0;
extern float multiplier;

#define IN_RANGE(n, min, max) ((x) >= (min) && (x) < (max))
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

int colors[5][3] = {
    {122, 31, 206},
    {229, 124 ,22},
    {79, 229, 22},
    {22, 221, 229},
    {16, 233, 110},
};

int i;

void Cycle_RGB(float mult, int delay){
    for (i = 0; i <= 4; i++){
    
    TCC3_REGS->TCC_CC[1] = RGB_to_CC(mult*colors[i][0]);
    TCC3_REGS->TCC_CC[0] = RGB_to_CC(mult*colors[i][1]);
    TCC3_REGS->TCC_CC[3] = RGB_to_CC(mult*colors[i][2]);
    delay_ms(delay);
    }
}
void test(void){
    delay_ms(1000);
    multiplier = 0.05f;
}

void Adjust_Brightness(void) {
    /*
     *  This is the main interrupt function for adjusting the brightness.
     *  Initial brightness is set at 50%. 
     *  Keep increasing the brightness by 20% until 90% of brightness has been achieved. Then, proceed to decrease by 30% until 30% of brightness have been achieved.
     */
    
    
    if (direction == 1){
        multiplier += 0.2f;
        if (multiplier >= 0.9f) {
            multiplier = 0.9f;
            direction = -1;
        }
    } else if (direction == -1){
        multiplier -= 0.3f;
        if (multiplier <= 0.3f){
            multiplier = 0.3f;
            direction = 1;
        }
    }
    /*
    if (brightness > INIT_TOP*0.1 & decreasing_brightness == 0 ) 
    { // If brightness is greater than 10% (less than 90% complement) of 3.3V, increase brightness by 20% by decrementing.
        brightness -= INIT_TOP*0.2; // Increase brightness
    } else if (brightness <= INIT_TOP*0.1 || decreasing_brightness == 1) 
        { // If brightness is less than 10% (greater  than 90% complement) of 3.3V
        brightness += INIT_TOP*0.3; // Decrease brightness by 30% by incrementing.
        decreasing_brightness = 1;
        if (brightness >= INIT_TOP*0.7) 
            
        { // Now if it's greater than 70% of 3.3V (less than 30% complement), we will no longer decrease the brightness.
            decreasing_brightness = 0; // Decrease brightness
        }
    }
    
    TCC3_REGS->TCC_CC[1] = brightness;
    TCC3_REGS->TCC_CC[4] = brightness;
    TCC3_REGS->TCC_CC[3] = brightness;
     */
}

void Adjust_Period_and_Direction(int adc_value){
        
        /* ADC Value: {%} * 2^10
         * 0-20%: 0-205.8
         * 20%-40%: 205.8 - 409.6
         * 40%-60%: 614.4
         * 60%-80%: 819.2
         * 80%-100%: 1024
         * To avoid floating point errors, rounded off the nearest whole number.
         */
        
        if (IN_RANGE(adc_value, 0, 206)){
            
        } else if (IN_RANGE(adc_value, 206, 410)){
            
        } else if (IN_RANGE(adc_value, 410, 614)){
            // Do Nothing
            asm("nop");
            
        } else if (IN_RANGE(adc_value, 614, 819)){
            
        } else if (IN_RANGE(adc_value, 819, 1024)){
            
        }
}

uint16_t RGB_to_CC(uint8_t x){
    return (uint16_t)(INIT_TOP * (1.0f - (float)x / 255.0f));
}
#endif
