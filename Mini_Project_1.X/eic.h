#ifndef EIC_H
#define EIC_H

#include "init_io.h"
#include "sw_fcns.h"
#include "adc.h"
#include "start.h"

extern float multiplier;

void __attribute__((interrupt())) EIC_EXTINT_0_Handler(void);
void __attribute__((interrupt())) EIC_EXTINT_1_Handler(void);
void EIC_Initialize(void);
void NVIC_Initialize(void);

void __attribute__((interrupt())) EIC_EXTINT_0_Handler(void) {
    /*
     * Handler for SW1 or PA00
     */
    EIC_SEC_REGS->EIC_INTFLAG |= (1 << 0);

    ADC_ConversionStart();
    while (!ADC_ConversionStatusGet());
    uint16_t adc_value = ADC_ConversionResultGet();


    Adjust_Brightness(adc_value);
}

void __attribute__((interrupt())) EIC_EXTINT_1_Handler(void) {
    /*
     * Handler for Sw2 or PA01
     */
    EIC_SEC_REGS->EIC_INTFLAG |= (1 << 1);

    ADC_ConversionStart();
    while (!ADC_ConversionStatusGet());
    uint16_t adc_value = ADC_ConversionResultGet();

    Adjust_Period_and_Direction(adc_value);
    EIC_SEC_REGS->EIC_INTFLAG |= (1 << 1);
}

void EIC_Initialize(void) {
    SW_Init();
    /* Reset and wait for the operation to finish */
    EIC_SEC_REGS->EIC_CTRLA |= 0x01; // Set SWRST bit to 1 to reset
    while ((EIC_SEC_REGS->EIC_SYNCBUSY & 0x01) == 0x01);

    /* For both EXTINT0 (SW0; PA00) and EXTINT1(SW1; PA01)  */
    // 28.7.10
    EIC_SEC_REGS -> EIC_CONFIG0 |= (0x2 << 0); // Falling Edge Detection with Filter Enabled: EXTINT[0]
    EIC_SEC_REGS -> EIC_CONFIG0 |= (1 << 3);
    EIC_SEC_REGS -> EIC_CONFIG0 |= (0x2 << 4); // Falling Edge Detection with Filter Enabled: EXTINT[1]
    EIC_SEC_REGS -> EIC_CONFIG0 |= (1 << 7);
    // 28.7.12
    EIC_SEC_REGS -> EIC_DEBOUNCEN |= (1 << 0); // Debounce Enable for EXTINT[0]
    EIC_SEC_REGS -> EIC_DEBOUNCEN |= (1 << 1); // Debounce Enable for EXTINT[1]

    /* Set prescaler to slow down clock from 4MHz to 15.625 kHz */
    EIC_SEC_REGS->EIC_DPRESCALER |= 0x000100FF;

    /* EXTINT 0 and 1 Enable */
    // 28.7.7
    EIC_SEC_REGS -> EIC_INTENSET |= (1 << 0); // EXTINT[0] 
    EIC_SEC_REGS -> EIC_INTENSET |= (1 << 1); // EXTINT[1]


    /* Enable EIC */
    // 28.7.1
    EIC_SEC_REGS -> EIC_CTRLA |= 0x02;
    while ((EIC_SEC_REGS->EIC_SYNCBUSY & 0x02) == 0x02);

    /* Clear INTFLAG for EXTINT 0 and 1*/
    // 28.7.7
    EIC_SEC_REGS -> EIC_INTFLAG |= (1 << 0); // EXTINT[0]
    EIC_SEC_REGS -> EIC_INTFLAG |= (1 << 1); // EXTINT[1]

}

void NVIC_Initialize(void) {

    /* Enable the interrupt sources and configure the priorities as configured
     * from within the "Interrupt Manager" of MHC. */

    __DMB();
    __enable_irq();

    NVIC_SetPriority(EIC_EXTINT_0_IRQn, 2); // Set priority for EXTINT0 to 3
    NVIC_EnableIRQ(EIC_EXTINT_0_IRQn); // Enables EXTINT0

    NVIC_SetPriority(EIC_EXTINT_1_IRQn, 2); // Set priority for EXTINT1 to 3
    NVIC_EnableIRQ(EIC_EXTINT_1_IRQn); // Enables EXTINT1
}


#endif