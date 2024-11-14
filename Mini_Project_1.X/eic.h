#ifndef EIC_H
#define EIC_H

#include "init_io.h"
#include "sw_fcns.h"

void __attribute__((interrupt())) EIC_EXTINT_2_Handler(void) {
    /*
 * During IRQ execution, the core is in Handler mode. Compared to
 * Thread mode, Handler mode is always privileged. Thus, secure access
 * to registers is always assumed inside the IRQ. 
 */
    // Read Potentiometer
    ADC_ConversionStart();
    while(!ADC_ConversionStatusGet());
    uint16_t adc_value = ADC_ConversionResultGet();

    // Switch 1
    if (EIC_SEC_REGS->EIC_INTFLAG |= (1 << 0)){
         EIC_SEC_REGS->EIC_INTFLAG |= (1 << 0);                      // Clears the interrupt flag to re-enable interrupt generation
         Adjust_Brightness();
    }
    
    // Switch 2
    else if (EIC_SEC_REGS->EIC_INTFLAG |= (1 << 1)){
        Adjust_Period_and_Direction(adc_value);
    }
}
 
void EIC_Initialize(void){
    
    /* Initializes SW1 and SW2  as an input */
    SW_Init(0, 0); // SW1
    SW_Init(0, 1); // SW2
    
    /* Reset and wait for the operation to finish */
    EIC_SEC_REGS->EIC_CTRLA = 0x01;                             // Set SWRST bit to 1 to reset
    while((EIC_SEC_REGS->EIC_SYNCBUSY & 0x01) == 0x01);
    
    /* For both EXTINT0 (SW0; PA00) and EXTINT1(SW1; PA01)  */
    // 28.7.10
    EIC_SEC_REGS->EIC_CONFIG0 =  0x99;  // 10011001
    // 28.7.12
    EIC_SEC_REGS->EIC_DEBOUNCEN  = 0x3;    // 11
    
    /* Set prescaler to slow down clock from 4MHz to 15.625 kHz */
    EIC_SEC_REGS->EIC_DPRESCALER = 0x000100FF; 
    
    /* EXTINT 0 and 1 Enable */
    // 28.7.7
    EIC_SEC_REGS->EIC_INTENSET = 0x3;                    
    
    
    /* Enable EIC */
    // 28.7.1
    EIC_SEC_REGS->EIC_CTRLA = 0x02;
    while((EIC_SEC_REGS->EIC_SYNCBUSY & 0x02) == 0x02);
    
    /* Clear INTFLAG for EXTINT 0 and 1*/
    // 28.7.7
    EIC_SEC_REGS->EIC_INTFLAG |= (1 << 1); 
    EIC_SEC_REGS->EIC_INTFLAG |= (1 << 0); 
}
 
void NVIC_Initialize( void )
{
    
    /* Enable the interrupt sources and configure the priorities as configured
     * from within the "Interrupt Manager" of MHC. */
    
    __DMB();
    __enable_irq();
    
    NVIC_SetPriority(EIC_EXTINT_0_IRQn, 3);                     // Set priority for EXTINT0 to 3
    NVIC_EnableIRQ(EIC_EXTINT_0_IRQn);                          // Enables EXTINT0
    
    NVIC_SetPriority(EIC_EXTINT_1_IRQn, 4);                     // Set priority for EXTINT1 to 4
    NVIC_EnableIRQ(EIC_EXTINT_1_IRQn);                          // Enables EXTINT1
}


#endif