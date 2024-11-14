#ifndef ADC_H
#define ADC_H

void ADC_Initialize(void){
    /* Reset ADC */
    ADC_REGS - > ADC_CTRLA = (1 < <0);
    while ((ADC_REGS - > ADC_SYNCBUSY & (1 < <0)) == (1 < <0));
    /* Prescaler */
    ADC_REGS - > ADC_CTRLB = (2 < <0);
    /* Sampling length */
    ADC_REGS - > ADC_SAMPCTRL = (3 <<0);
    /* Reference */
    ADC_REGS - > ADC_REFCTRL = (0x5 <<0); // AVDD w/o multiplier
    /* Input pin */
    ADC_REGS - > ADC_INPUTCTRL = (0 << 0);
    /* Resolution & Operation Mode */
    ADC_REGS - > ADC_CTRLC = (uint16_t)((0x2 << 4) | (0 << 8)); // 10 Bits
    /* Clear all interrupt flags */
    ADC_REGS - > ADC_INTFLAG = (uint8_t)0x07;
    while (0 U != ADC_REGS - > ADC_SYNCBUSY);

}

#endif