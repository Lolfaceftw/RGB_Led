#ifndef PIN_CALCULATION_H
#define PIN_CALCULATION_H

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

#endif