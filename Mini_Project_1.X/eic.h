#ifndef EIC_H
#define EIC_H

#include "init_io.h"

void __attribute__((interrupt())) EIC_EXTINT_2_Handler(void) {
    /*
 * During IRQ execution, the core is in Handler mode. Compared to
 * Thread mode, Handler mode is always privileged. Thus, secure access
 * to registers is always assumed inside the IRQ. 
 */
    
    while(PORT_SEC_REGS -> GROUP[0].PORT_IN & (0 << 23));
    if (EIC_SEC_REGS->EIC_INTFLAG |= (1 << 2)){
         EIC_SEC_REGS->EIC_INTFLAG |= (1 << 2);                      // Clears the interrupt flag to re-enable interrupt generation
         Adjust_Brightness();
    }
  
}
 
void EIC_Initialize(void){
    
    /* Initializes SW1 and SW2  as an input */
    SW_Init(0, 0); // SW1
    SW_Init(0, 1); // SW2
    
    /* Reset and wait for the operation to finish */
    EIC_SEC_REGS->EIC_CTRLA = 0x01;                             // Set SWRST bit to 1 to reset
    while((EIC_SEC_REGS->EIC_SYNCBUSY & 0x01) == 0x01);
    
    /* Interrupt sense type, filter, and debouncing control for EXTINT2 */
    EIC_SEC_REGS->EIC_CONFIG0 =  0x00000A00;                    // Configured for Falling Edge Detection with Filter enabled
    EIC_SEC_REGS->EIC_DEBOUNCEN  = 0x00000004;                  // Enable debouncer for EXTINT2
    EIC_SEC_REGS->EIC_DPRESCALER = 0x000100FF;                  // Set prescaler to slow down clock from 4MHz to 15.625 kHz
    
    /* External Interrupt enable for EXTINT2 */
    EIC_SEC_REGS->EIC_INTENSET = 0x00000004;                    // Enables EXTINT2
    EIC_SEC_REGS->EIC_CTRLA = 0x02;                             // Enables EIC
    while((EIC_SEC_REGS->EIC_SYNCBUSY & 0x02) == 0x02);
    
    EIC_SEC_REGS->EIC_INTFLAG |= (1 << 2);                      // Clear INTFLAG
}
 
void NVIC_Initialize( void )
{
    
    /* Enable the interrupt sources and configure the priorities as configured
     * from within the "Interrupt Manager" of MHC. */
    
    __DMB();
    __enable_irq();
    
    NVIC_SetPriority(EIC_EXTINT_2_IRQn, 3);                     // Set priority for EXTINT2 to 3
    NVIC_EnableIRQ(EIC_EXTINT_2_IRQn);                          // Enables EXTINT2
    
}


#endif