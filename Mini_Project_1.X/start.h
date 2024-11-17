#ifndef START_H
#define START_H

#include "adc.h"
#include "clk.h"

void Start(void);
void Cloks_Init(void);
void IO_Init(void);
void SW_Init(void);
void EIC_Start(void);

void Start(void) {
    /*
     * Initializes the clock, pin, ADC, and EIC.
     * 
     *  @return: void
     */

    Clocks_Init();

    IO_Init();
    EIC_Start();

}

void Clocks_Init(void) {
    // Initialize the Clocks
    GCLK_Init();
    Clock_Source();
    ADC_Initialize();
    ADC_Enable();

    // Initialize TCC3
    TC0_Init();
    TCC3_Init();
}

void IO_Init(void) {
    // Initialize the RGB pins
    PA_03_Init();
    PA_06_Init();
    PB_03_Init();

    // Initialize Potentiometer Input
    PB_02_Init();
}

void SW_Init(void) {
    // Initialize SW and SW2
    SW_1_Init();
    SW_2_Init();
}

void EIC_Start(void) {
    /* Enable the EIC peripheral clock */
    MCLK_REGS->MCLK_APBAMASK |= MCLK_APBAMASK_EIC_Msk;

    //GCLK_REGS -> GCLK_PCHCTRL[26] = (1 << 6); // Bit 6 Enable
    //while ((GCLK_REGS -> GCLK_PCHCTRL [26] * (1 << 6)) == 0);

    /* To enable the filter and debouncer in EIC, the GCLK_EIC should be enabled */
    GCLK_REGS->GCLK_PCHCTRL[4] = 0x00000040;

    EIC_Initialize();
    NVIC_Initialize();
}
#endif