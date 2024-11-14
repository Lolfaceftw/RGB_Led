#ifndef INIT_IO_H
#define INIT_IO_H

#include "pin_calculation.h"

void PB_02_Init(int group_number, int pin_number){
     /*
     *  Initializes PB02 acting as the potentiometer input.
     * 
     *  Group 1: Address Spacing 0x80
     *  Target: PMUX Enabled; ADC Selected
     */
    PORT_SEC_REGS -> GROUP[group_number].PORT_PINCFG[pin_number] = 0x1U; // PMUX EN
    PORT_SEC_REGS -> GROUP[group_number].PORT_PMUX[pin_number] = 0x1U; // Peripheral B
}

void PA_04_Init(int group_number, int pin_number){
       /*
     *  Initializes PB02 acting as the potentiometer input.
     * 
     *  Group 1: Address Spacing 0x80
     *  Target: PMUX Enabled; ADC Selected
     */
    PORT_SEC_REGS -> GROUP[pin_number].PORT_PINCFG[pin_number] = 0x1U; // PMUX EN
    PORT_SEC_REGS -> GROUP[pin_number].PORT_PMUX[pin_number] = 0x1U; // Peripheral B
}
void PA_03_Init(int group_number, int pin_number) {    
    /*
     *  Initializes PA03 acting as the R channel, active-HI.
     * 
     *  Group 0: Address Spacing 0x00
     *  Target: Output; input enabled.
     *  DIR: 1, INEN: 1, PULLEN: X, OUT: X
     */
    // 31.7.1
    PORT_SEC_REGS->GROUP[group_number].PORT_DIRSET = (1 << pin_number); // Set as output
    // 31.7.6
    PORT_SEC_REGS->GROUP[group_number].PORT_OUTCLR = (1 << pin_number); // Reset Pin
    // 31.7.14
    PORT_SEC_REGS->GROUP[group_number].PORT_PINCFG[pin_number] = 0x3; // Enables INEN and PMUXEN, output with input enabled.
    // 31.7.13
    PORT_SEC_REGS->GROUP[group_number].PORT_PMUX[PMUX_pin(pin_number)] = (0x90 << 4); // J Peripheral for PA03, PMUXO[3:0], required PMUXEN 1
}

void PA_02_Init(int group_number, int pin_number) {    
    /*
     *  Initializes PA02 acting as the G channel, active-HI.
     * 
     *  Group 0: Address Spacing 0x00
     *  Target: Output; input enabled.
     *  DIR: 1, INEN: 1, PULLEN: X, OUT: X[
     */
    // 31.7.1
    PORT_SEC_REGS->GROUP[group_number].PORT_DIRSET = (1 << pin_number); // Set as output
    // 31.7.6
    PORT_SEC_REGS->GROUP[group_number].PORT_OUTCLR = (1 << pin_number); // Reset Pin
    // 31.7.14
    PORT_SEC_REGS->GROUP[group_number].PORT_PINCFG[pin_number] = 0x3; // Enables INEN and PMUXEN, output with input enabled.
    // 31.7.13
    PORT_SEC_REGS->GROUP[group_number].PORT_PMUX[PMUX_pin(pin_number)] = (0x90 << 0); // J Peripheral for PA02, PMUXE[3:0], required PMUXEN 1
}

void PB_03_Init(int group_number, int pin_number) {    
    /*
     *  Initializes PB03 acting as the B channel, active-HI.
     * 
     *  Group 1: Address Spacing 0x80
     *  Target: Output; input enabled.
     *  DIR: 1, INEN: 1, PULLEN: X, OUT: X
     */
    // 31.7.1
    PORT_SEC_REGS->GROUP[group_number].PORT_DIRSET = (1 << pin_number); // Set as output
    // 31.7.6
    PORT_SEC_REGS->GROUP[group_number].PORT_OUTCLR = (1 << pin_number); // Reset Pin, set to low output drive level.
    // 31.7.14
    PORT_SEC_REGS->GROUP[group_number].PORT_PINCFG[pin_number] = 0x3; // Enables INEN and PMUXEN, output with input enabled.
    // 31.7.13
    PORT_SEC_REGS->GROUP[group_number].PORT_PMUX[PMUX_pin(pin_number)] = (0x90 << 4); // J Peripheral for PB03, PMUXO[3:0], required PMUXEN 1
}


void SW_Init(int group_number, int pin_number){
    /*
     *  Initializes SW1 and SW2 as input with pull-up configuration, active-LO. Enables the EIC Peripheral as well.
     *  Group 0: Address Spacing 0x00
     *  Target: Input with pull-up
     *  DIR: 0, INEN: 1, PULLEN: 1, OUT: 1
     */
    // 31.7.1
    PORT_SEC_REGS->GROUP[group_number].PORT_DIRSET = (0 << pin_number); // Set as input.
    // 31.7.14
    PORT_SEC_REGS->GROUP[group_number].PORT_DIRSET = 0x7; // Enables PULLEN, INEN, and PMUXEN, input with pull.
    // 31.7.6
    PORT_SEC_REGS->GROUP[group_number].PORT_OUTSET= (1 << pin_number); // Set as internal pull-up.
    // 31.7.13
    if (is_even(pin_number) == 1) {
        PORT_SEC_REGS->GROUP[1].PORT_PMUX[PMUX_pin(pin_number)] = (0x0 << 0); // J Peripheral for PB02, PMUXE[3:0], required PMUXEN 1
    } else{
        PORT_SEC_REGS->GROUP[1].PORT_PMUX[PMUX_pin(pin_number)] = (0x0 << 4); // J Peripheral for PB02, PMUXO[3:0], required PMUXEN 1
    }
}

#endif