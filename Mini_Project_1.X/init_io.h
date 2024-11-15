#ifndef INIT_IO_H
#define INIT_IO_H

#include "adc.h"
#include "clk.h"

extern volatile unsigned int brightness;

void Start(void){
    /*
     * Initializes the clock, pin, ADC, and EIC.
     * 
     *  @return: void
     */
    // Initialize the Clocks
    GCLK_Init();
    Clock_Source();
    ADC_Initialize();
    ADC_Enable();
    
    // Initialize TCC3
    TCC3_Init(brightness);
    
    // Initialize the RGB pins
    PA_03_Init();
    PA_06_Init();
    PB_03_Init();
    
    // Initialize Potentiometer Input
    PB_02_Init();
    
    // Initialize SW and SW2
    SW_1_Init();
    SW_2_Init();
    
    /* Enable the EIC peripheral clock */
    MCLK_REGS->MCLK_APBAMASK |= MCLK_APBAMASK_EIC_Msk;

    GCLK_REGS -> GCLK_PCHCTRL[23] = (1 << 6); // Bit 6 Enable
    while ((GCLK_REGS -> GCLK_PCHCTRL [23] * (1 << 6)) == 0);
    
    /* To enable the filter and debouncer in EIC, the GCLK_EIC should be enabled */
    GCLK_REGS->GCLK_PCHCTRL[4] = 0x00000040;
    
    EIC_Initialize();
    NVIC_Initialize();
}
void PB_02_Init(void){
     /*
     *  Initializes PB02 acting as the potentiometer input.
     * 
     *  Group 1: Address Spacing 0x80
     *  Target: PMUX Enabled; ADC Selected
     */
    PORT_SEC_REGS -> GROUP[1].PORT_PINCFG[1] = 0x1U; // PMUX EN
    PORT_SEC_REGS -> GROUP[1].PORT_PMUX[1] = 0x1U; // Peripheral B
}

void PA_03_Init(void) {    
    /*
     *  Initializes PA03 acting as the R channel, active-HI.
     * 
     *  Group 0: Address Spacing 0x00
     *  Target: Output; input enabled.
     *  DIR: 1, INEN: 1, PULLEN: X, OUT: X
     */
    // 31.7.1
    PORT_SEC_REGS->GROUP[0].PORT_DIRSET = (1 << 3); // Set as output
    // 31.7.6
    PORT_SEC_REGS->GROUP[0].PORT_OUTCLR = (1 << 3); // Reset Pin
    // 31.7.14
    PORT_SEC_REGS->GROUP[0].PORT_PINCFG[3] = 0x3; // Enables INEN and PMUXEN, output with input enabled.
    // 31.7.13
    PORT_SEC_REGS->GROUP[0].PORT_PMUX[1] = (0x90 << 4); // J Peripheral for PA03, PMUXO[3:0], required PMUXEN 1
}

void PA_06_Init(void) {    
    /*
     *  Initializes PA06 acting as the G channel, active-HI.
     * 
     *  Group 0: Address Spacing 0x00
     *  Target: Output; input enabled.
     *  DIR: 1, INEN: 1, PULLEN: X, OUT: X
     */
    // 31.7.1
    PORT_SEC_REGS->GROUP[0].PORT_DIRSET = (1 << 6); // Set as output
    // 31.7.0
    PORT_SEC_REGS->GROUP[0].PORT_OUTCLR = (1 << 6); // Reset Pin
    // 31.7.14
    PORT_SEC_REGS->GROUP[0].PORT_PINCFG[6] = 0x3; // Enables INEN and PMUXEN, output with input enabled.
    // 31.7.13
    PORT_SEC_REGS->GROUP[0].PORT_PMUX[1] = (0x90 << 0); // J Peripheral for PA06, PMUXE[3:0], required PMUXEN 1
}

void PB_03_Init(void) {    
    /*
     *  Initializes PB03 acting as the B channel, active-HI.
     * 
     *  Group 1: Address Spacing 0x80
     *  Target: Output; input enabled.
     *  DIR: 1, INEN: 1, PULLEN: X, OUT: X
     */
    // 31.7.1
    PORT_SEC_REGS->GROUP[1].PORT_DIRSET = (1 << 3); // Set as output
    // 31.7.6
    PORT_SEC_REGS->GROUP[1].PORT_OUTCLR = (1 << 3); // Reset Pin, set to low output drive level.
    // 31.7.14
    PORT_SEC_REGS->GROUP[1].PORT_PINCFG[3] = 0x3; // Enables INEN and PMUXEN, output with input enabled.
    // 31.7.13
    PORT_SEC_REGS->GROUP[1].PORT_PMUX[1] = (0x90 << 4); // J Peripheral for PB03, PMUXO[3:0], required PMUXEN 1
}


void SW_1_Init(void){
    /*
     *  Initializes SW1 (PA00) as input with pull-up configuration, active-LO. Enables the EIC Peripheral as well.
     *  Group 0: Address Spacing 0x00
     *  Target: Input with pull-up
     *  DIR: 0, INEN: 1, PULLEN: 1, OUT: 1
     */
    // 31.7.1
    PORT_SEC_REGS->GROUP[0].PORT_DIRSET = (0 << 0); // Set as input.
    // 31.7.14
    PORT_SEC_REGS->GROUP[0].PORT_PINCFG[0] = 0x7; // Enables PULLEN, INEN, and PMUXEN, input with pull.
    // 31.7.6
    PORT_SEC_REGS->GROUP[0].PORT_OUTSET= (1 << 0); // Set as internal pull-up.
    // 31.7.13
    PORT_SEC_REGS->GROUP[0].PORT_PMUX[0] = (0x0 << 0); // A Peripheral for PA00, PMUXE[3:0], required PMUXEN 1
}

void SW_2_Init(void){
    /*
     *  Initializes SW2 (PA01) as input with pull-up configuration, active-LO. Enables the EIC Peripheral as well.
     *  Group 0: Address Spacing 0x00
     *  Target: Input with pull-up
     *  DIR: 0, INEN: 1, PULLEN: 1, OUT: 1
     */
    // 31.7.1
    PORT_SEC_REGS->GROUP[0].PORT_DIRSET = (0 << 1); // Set as input.
    // 31.7.14
    PORT_SEC_REGS->GROUP[0].PORT_PINCFG[1] = 0x7; // Enables PULLEN, INEN, and PMUXEN, input with pull.
    // 31.7.6
    PORT_SEC_REGS->GROUP[0].PORT_OUTSET= (1 << 1); // Set as internal pull-up.
    // 31.7.13
    PORT_SEC_REGS->GROUP[0].PORT_PMUX[0] = (0x0 << 4); // A Peripheral for PA01, PMUXO[3:0], required PMUXEN 1
}

#endif