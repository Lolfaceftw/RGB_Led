#ifndef CLK_H
#define CLK_H

#define INIT_TOP 468 // Let x be the PER value, $100=\frac{48e6}{1024(x+1)}$

void GCLK_Init(void){
    /* Initialize Clocks */
    // Main Clock enabled by default
    GCLK_REGS -> GCLK_PCHCTRL[25] = 0x00000040;
    while ((GCLK_REGS -> GCLK_PCHCTRL [25] & 0x00000040) == 0) ; // Wait for synchronization
    }
 
void TCC3_Init(int brightness) {
    /* Reset TCC */
    TCC3_REGS->TCC_CTRLA = 0x01; // Set SWRST bit to 1 to reset
    while(TCC3_REGS->TCC_SYNCBUSY & ~(1<<0)); // Wait for synchronization
    
    /* Clock Prescaler and Mode */    
    TCC3_REGS->TCC_CTRLA = (1 << 12) | (7 << 8); // Precsync = PRESC | Prescaler = 1024
    
    TCC3_REGS->TCC_WEXCTRL = TCC_WEXCTRL_OTMX(0UL); // Default configuration
    TCC3_REGS->TCC_WAVE = (2 << 0) | (0 << 4) | (1<<17);  // 0x2 NPWM Normal PWM PER TOP/Zero or Single slope PWM
                                                // RAMP 1 operation (Polarity 1) - bit 16, set at CCx, clear at TOP 
    
    /* Configure duty cycle values */    
    TCC3_REGS->TCC_PER = INIT_TOP; // Set the period value
    
    /*
     * PA03 - TCC3/WO[1]
     * PA06 - TCC3/WO[4]
     * PB03 - TCC3/WO[3]
     */
    
    /* Set the duty cycle or brightness @ default 50% */
    // PA03
    TCC3_REGS->TCC_CC[1] = brightness;
    
    // PA06
    TCC3_REGS->TCC_CC[4] = brightness;
    
    // PB03
    TCC3_REGS->TCC_CC[3] = brightness;  
    /* TCC enable */
    TCC3_REGS->TCC_CTRLA |= (1 << 1); // Enables TCC
    while(TCC3_REGS->TCC_SYNCBUSY & ~(1<<1)); // Wait for synchronization
    
}
 
void Clock_Source(void){
    
    /*
     * The chip starts in PL0 ("Energy Saving"); as a result, the chip is also
     * limited in the maximum frequency it can support (12 vs 48 MHz). Switch
     * it to PL2 ("Performance") before doing other Clock_Source() tasks.
     * 
     * - Berts
     */
    PM_REGS->PM_INTFLAG = 0x01;
    PM_REGS->PM_PLCFG = 0x02;
    while ((PM_REGS->PM_INTFLAG & 0x01) == 0)
        asm("nop");
    PM_REGS->PM_INTFLAG = 0x01;
    
    /****************** Controls Initialization  *********************/    
    NVMCTRL_SEC_REGS->NVMCTRL_CTRLB = (2<<1);    
    SUPC_REGS->SUPC_VREGPLL = (1 << 1);
    
    /****************** DFLL Initialization  *********************/
    OSCCTRL_REGS->OSCCTRL_DFLLCTRL = 0;
    while((OSCCTRL_REGS->OSCCTRL_STATUS & (1<<24)) != (1<<24));  /* Waiting for the Ready state */
 
    /*Load Calibration Value*/
    uint8_t calibCoarse = (uint8_t)((((uint32_t)0x00806020U) >> 25U ) & 0x3fU);
    OSCCTRL_REGS->OSCCTRL_DFLLVAL = OSCCTRL_DFLLVAL_COARSE((uint32_t)calibCoarse) | OSCCTRL_DFLLVAL_FINE((uint32_t)512U);
    while((OSCCTRL_REGS->OSCCTRL_STATUS & (1<<24)) != (1<<24)); /* Waiting for the Ready state */
 
    /* Configure DFLL    */
    OSCCTRL_REGS->OSCCTRL_DFLLCTRL = (1<<1) ;
    while((OSCCTRL_REGS->OSCCTRL_STATUS & (1<<24)) != (1<<24)); /* Waiting for DFLL to be ready */
    
    /****************** GCLK 0 Initialization  *********************/    
    GCLK_REGS->GCLK_GENCTRL[0] = (1<<16) | (7<<0) | (1<<8);
    //while(GCLK_REGS->GCLK_SYNCBUSY & ~(1<<2));
    while(GCLK_REGS->GCLK_SYNCBUSY & (1<<2))
        asm("nop");
    
    // ADC Bus Clock: Generic Clock Generator Value | Channel Enable
    GCLK_REGS -> GCLK_PCHCTRL[28] = (0 << 0) | (1 << 6);
    while((GCLK_REGS -> GCLK_PCHCTRL[28] & (1 << 6)) != (1 << 6));
}

void TC0_Init(void){
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
}
#endif