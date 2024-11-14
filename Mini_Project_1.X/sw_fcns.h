#ifndef SW_FCNS_H
#define SW_FCNS_H

extern volatile unsigned int brightness;
volatile unsigned int decreasing_brightness = 0;

void Adjust_Brightness(void) {
    /*
     *  This is the main interrupt function for adjusting the brightness.
     *  Initial brightness is set at 50%. 
     *  Keep increasing the brightness by 20% until 90% of brightness has been achieved. Then, proceed to decrease by 30% until 30% of brightness have been achieved.
     */
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
}
#endif