#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdbool.h>
#include <math.h>
#include "eic.h"
#include "init_io.h"
#include "clk.h"
#include "adc.h"

/*
 * Initialization
 */
 
#define INIT_TOP 468 // Let x be the PER value, $100=\frac{48e6}{1024(x+1)}$
volatile unsigned int x;
#define IN_RANGE(n, min, max) ((x) >= (min) && (x) < (max))

volatile unsigned int brightness = 234; // Initial of 50% brightness.
volatile unsigned int decreasing_brightness = 0;

volatile int read_count(){
    // Allow read access of COUNT register
    // Return back the counter value
    TC0_REGS -> COUNT16.TC_CTRLBSET = (0x4 << 5);
    return TC0_REGS -> COUNT16.TC_COUNT; // 39.8.13
    
}
 
int main(int argc, char** argv) {
    // Initialize the Clocks
    GCLK_Init();
    Clock_Source();
    ADC_Initialize();
    ADC_Enable();
    
    // Initialize TCC3
    TCC3_Init(brightness);
    
    // Initialize the pins
    PA_04_Init(0,4);
    PA_03_Init(0,3);
    PA_02_Init(0,6);
    
    PB_03_Init(1,3);
    PB_02_Init(1,2);
    
    
    // END
    /* Enable the EIC peripheral clock */
    MCLK_REGS->MCLK_APBAMASK |= MCLK_APBAMASK_EIC_Msk;
     // Enable the TC0 Bus Clock
    GCLK_REGS -> GCLK_PCHCTRL[23] = (1 << 6); // Bit 6 Enable
    while ((GCLK_REGS -> GCLK_PCHCTRL [23] * (1 << 6)) == 0);
    
    TC0_Init();
    
    /* To enable the filter and debouncer in EIC, the GCLK_EIC should be enabled */
    GCLK_REGS->GCLK_PCHCTRL[4] = 0x00000040;
    
    EIC_Initialize();
    NVIC_Initialize();
    
    while (1) {
        
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
         
         */
        
        if (IN_RANGE(adc_value, 0, 205.8)){
            
        } else if (IN_RANGE(adc_value, 205.8, 409.6)){
            
        } else if (IN_RANGE(adc_value, 409.6, 614.4)){
            
        } else if (IN_RANGE(adc_value, 614.4, 819.2)){
            
        } else if (IN_RANGE(adc_value, 819.2, 1024)){
            
        }
        
         
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
        }
}
        
    return (EXIT_SUCCESS);
}

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
}
