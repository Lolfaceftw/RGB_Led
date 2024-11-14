#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdbool.h>
#include <math.h>
#include "eic.h"
#include "init_io.h"
#include "clk.h"
#include "adc.h"

volatile unsigned int x;
volatile unsigned int brightness = 234; // Initial of 50% brightness.

#define INIT_TOP 468 // Let x be the PER value, $100 Hz=\frac{48e6}{1024(x+1)}$
#define IN_RANGE(n, min, max) ((x) >= (min) && (x) < (max))

volatile int read_count(){
    // Allow read access of COUNT register
    // Return back the counter value
    TC0_REGS -> COUNT16.TC_CTRLBSET = (0x4 << 5);
    return TC0_REGS -> COUNT16.TC_COUNT; // 39.8.13
    
}
 
int main(int argc, char** argv) {
    Start();
    while (1) {
        // Do nothing when SW1 or SW2 is not pressed.
        while((EIC_SEC_REGS->EIC_INTFLAG |= (0 << 0)) | (EIC_SEC_REGS->EIC_INTFLAG |= (0 << 1)));
        
        // Read Potentiometer
        ADC_ConversionStart();
        while(!ADC_ConversionStatusGet());
        uint16_t adc_value = ADC_ConversionResultGet();
        
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
        
        /*
        // If we're in the first half of the 1s period, keep TCC enabled and use brightness
        if (read_count() < TC0_REGS->COUNT16.TC_CC[0] / 2) {
            // Re-enable TCC if it's disabled
            if (!(TCC3_REGS->TCC_CTRLA & (1 << 1))) {
                TCC3_REGS->TCC_CTRLA |= (1 << 1);  // Enable TCC
                while (TCC3_REGS->TCC_SYNCBUSY & (1 << 1));  // Wait for synchronization
            }

            TCC3_REGS->TCC_CC[1] = brightness;  // Set the current brightness
        }
        // If we're in the second half of the 1s period, disable TCC
        else {
            if (TCC3_REGS->TCC_CTRLA & (1 << 1)) {
                TCC3_REGS->TCC_CTRLA &= ~(1 << 1);  // Disable TCC
                while (TCC3_REGS->TCC_SYNCBUSY & (1 << 1));  // Wait for synchronization
            }
        }  */
}
        
    return (EXIT_SUCCESS);
}


