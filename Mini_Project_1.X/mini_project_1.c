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
 
/*
void GCLK_Init(void);
void Clock_Source(void);
void PA_03_Init(void); // "R" Channel, active-HI
void PA_06_Init(void); // "G" Channel, active-HI
void PB_03_Init(void); // "B" Channel, active-HI
void PB_02_Init(void); // Potentiometer Input
void SW_Init(void); // Initializes the switches
void TCC3_Init(void);
void Adjust_Brightness(void);

// Interrupts
void __attribute__((interrupt())) EIC_EXTINT_2_Handler(void);
void EIC_Initialize(void);
void NVIC_Initialize(void);
// End Interrupts


*/

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
    
    // Setting up the TC 0 -> CTRLA Register
    TC0_REGS -> COUNT16.TC_CTRLA = (1); // Software Reset; Bit 0
    while(TC0_REGS -> COUNT16.TC_SYNCBUSY & (1));
    
    TC0_REGS -> COUNT16.TC_CTRLA = (0x0 << 2); // Set to 16 bit mode; Bit[3:2].
    TC0_REGS -> COUNT16.TC_CTRLA = (0x1 << 4); // Reset counter on next prescaler clock Bit[5:4]]
    TC0_REGS -> COUNT16.TC_CTRLA = (0x7 << 8); // Prescaler Factor: 1024 Bit[10:8]]
    
    // Setting up the WAVE Register
    TC0_REGS -> COUNT16.TC_WAVE = (0x1 << 0); // Use MFRQ Bit [1:0]
    
    // Setting the Top Value
    TC0_REGS -> COUNT16.TC_CC[0] = 0x5B8D; // Set CC0 Top Value = 46875 for 48MHz
    
    TC0_REGS -> COUNT16.TC_CTRLA |= (1 << 1); // Enable TC0 Peripheral Bit 1
    
    /* To enable the filter and debouncer in EIC, the GCLK_EIC should be enabled */
    GCLK_REGS->GCLK_PCHCTRL[4] = 0x00000040;
    
    EIC_Initialize();
    NVIC_Initialize();
    
    while (1) {
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
