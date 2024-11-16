#ifndef ADC_H
#define ADC_H

void ADC_Initialize(void);
void ADC_Enable(void);
void ADC_ConversionStart(void);
bool ADC_ConversionStatusGet(Void);
void delay_ms(int delay);

void ADC_Initialize(void){
    /* Reset ADC */
    ADC_REGS -> ADC_CTRLA |= (1 <<0);
    while ((ADC_REGS -> ADC_SYNCBUSY & (1 <<0)) == (1 <<0));
    /* Prescaler */
    ADC_REGS -> ADC_CTRLB |= (2 <<0);
    /* Sampling length */
    ADC_REGS -> ADC_SAMPCTRL |= (3 <<0);
    /* Reference */
    ADC_REGS -> ADC_REFCTRL |= (0x5 <<0); // AVDD w/o multiplier
    /* Input pin */
    ADC_REGS -> ADC_INPUTCTRL |= (0 << 0);
    /* Resolution & Operation Mode */
    ADC_REGS -> ADC_CTRLC = (uint16_t)((0x2 << 4) | (0 << 8)); // 10 Bits
    /* Clear all interrupt flags */
    ADC_REGS -> ADC_INTFLAG |= (uint8_t)0x07;
    while (0U != ADC_REGS -> ADC_SYNCBUSY);

}

/* Enable ADC module */
void ADC_Enable(void) {
    ADC_REGS -> ADC_CTRLA |= (1 <<1);
    while (0U != ADC_REGS -> ADC_SYNCBUSY);
}

/* Start the ADC conversion by SW */
void ADC_ConversionStart(void) {
    ADC_REGS -> ADC_SWTRIG |= (1 <<1);
    while ((ADC_REGS -> ADC_SYNCBUSY & (1 <<10)) == (1 <<10));
}

// Read Conversion Result
uint16_t ADC_ConversionResultGet ( void )
{
return (uint16_t) ADC_REGS -> ADC_RESULT ;
}

/* Check whether result is ready */
bool ADC_ConversionStatusGet(void) {
    bool status;
    status = (((ADC_REGS -> ADC_INTFLAG & (1 <<0)) >> 0) != 0U);
    if (status == true) {
        ADC_REGS -> ADC_INTFLAG = (1 <<0);
    }
    return status;
}

void delay_ms(int delay) {
    int i;
    for (; delay > 0; delay --) {
        for (i = 0; i < 2657; i++);
    }
}
#endif