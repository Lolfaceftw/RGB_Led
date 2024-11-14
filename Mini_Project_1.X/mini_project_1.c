#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdbool.h>
#include <math.h>
/*
 * Initialization
 */
 
#define INIT_TOP 468 // Let x be the PER value, $100=\frac{48e6}{1024(x+1)}$
 
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
void PB_Initialize(void);
void EIC_Initialize(void);
void NVIC_Initialize(void);
// End Interrupts
 
volatile unsigned int state;
volatile unsigned int multiplier;
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
    // Initialize TCC3
    TCC3_Init();
    // Initialize the pins
    SW_Init(0, 0);
    SW_Init(0, 1);
    PA_03_Init(0,3);
    PA_06_Init(0,6);
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
    if (brightness > INIT_TOP*0.1 & decreasing_brightness == 0 ) { // If brightness is greater than 10% (less than 90% complement) of 3.3V, increase brightness by 20% by decrementing.
        brightness -= INIT_TOP*0.2; // Increase brightness
    } else if (brightness <= INIT_TOP*0.1 || decreasing_brightness == 1){ // If brightness is less than 10% (greater  than 90% complement) of 3.3V
        brightness += INIT_TOP*0.3; // Decrease brightness by 30% by incrementing.
        decreasing_brightness = 1;
        if (brightness >= INIT_TOP*0.7){ // Now if it's greater than 70% of 3.3V (less than 30% complement), we will no longer decrease the brightness.
            decreasing_brightness = 0; // Decrease brightness
        }
    }
    TCC3_REGS->TCC_CC[1] = brightness;
}
 
// --------------------------------------------------------------------------------------
 
// Interrupt Function
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
    
    /* Initializes PA23 as an input */
    PB_Initialize();
    
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
 
void PB_Initialize(void){
    /* Sets PA23 as an input with pull-down configuration */
    PORT_SEC_REGS->GROUP[0].PORT_DIRCLR = (1 << 23);
    PORT_SEC_REGS->GROUP[0].PORT_PINCFG[23] = 0x03;             // Enables INEN and PMUX
    PORT_SEC_REGS->GROUP[0].PORT_PMUX[23] = (0x0 << 4);         // Enables Interrupt for PA23
}
 
// End Interrupt Functions
int is_even(int pin){
    /*
     *  Checks if the pin number is even.
     *  @pin: The pin number to check.
     *  @return: Returns whether if it's even, 1, or odd, 0.
     */
    if (pin % 2 == 0){
        return 1;
        }
    else {
        return 0;
        }
    }

int PMUX_pin(int pin_number){
    /*
     * Calculates the right PMUX pin to use.
     * @pin_number: The pin number of PA or PB.
     * @return: Returns 2n+1 if it's odd and 2n if it's even.
     */
    
    if (is_even(pin_number) == 0){
        return (pin_number - 1) / 2;
        } else {
        return pin_number / 2;
            }
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
    PORT_SEC_REGS->GROUP[group_number].PORT_PMUX[1] = (0x90 << 4); // J Peripheral for PA03, PMUXO[3:0], required PMUXEN 1
}

void PA_02_Init(int group_number, int pin_number) {    
    /*
     *  Initializes PA02 acting as the G channel, active-HI.
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

void PB_02_Init(int group_number, int pin_number){
    /*
     *  Initializes PB02 as input from the potentiometer. When the switch is pressed, it must take input from the potentiometer; thus, we use pull-down.
     *  Group 1: Address Spacing 0x80
     *  Target: Input with pull-down
     *  DIR: 0, INEN: 1, PULLEN: 1, OUT: 0
     */
    // 31.7.1
    PORT_SEC_REGS->GROUP[group_number].PORT_DIRSET = (0 << pin_number); // Set as input
    // 31.7.14
    PORT_SEC_REGS->GROUP[group_number].PORT_PINCFG[pin_number] = 0x7; // Enables PULLEN, INEN, and PMUXEN, input with pull.
    // 31.7.6
    PORT_SEC_REGS->GROUP[group_number].PORT_OUTCLR = (1 << pin_number); // Set as internal pull-down since PULLEN: 1 and INEN: 1
    // 31.7.13
    PORT_SEC_REGS->GROUP[group_number].PORT_PMUX[PMUX_pin(pin_number)] = (0x90 << 0); // J Peripheral for PB02, PMUXE[3:0], required PMUXEN 1
    
    }

void SW_Init(int group_number, int pin_number){
    /*
     *  Initializes SW1 and SW2 as input with pull-up configuration, active-LO.
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
    PORT_SEC_REGS->GROUP[1].PORT_PMUX[PMUX_pin(pin_number)] = (0x90 << 0); // J Peripheral for PB02, PMUXE[3:0], required PMUXEN 1
    }


void GCLK_Init(void){
    /* Initialize Clocks */
    // Main Clock enabled by default
    GCLK_REGS -> GCLK_PCHCTRL[25] = 0x00000040;
    while ((GCLK_REGS -> GCLK_PCHCTRL [25] & 0x00000040) == 0) ; // Wait for synchronization
    }
 
void TCC3_Init(void) {
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
    TCC3_REGS->TCC_CC[1] = brightness; // Set the capture / compare register 0 or counter value       
    
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
}