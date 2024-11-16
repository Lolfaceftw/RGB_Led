#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <stdbool.h>
#include <math.h>
#include "eic.h"
#include "init_io.h"
#include "clk.h"
#include "adc.h"
#include "sw_fcns.h"
#include "start.h"

void Start(void);
//volatile unsigned int brightness = 234; // Initial of 50% brightness.
float multiplier = 0.1f;
int delay = 400;

int main(int argc, char** argv) {

    Start();
    while (1) {
        Cycle_RGB(multiplier, delay);
}
        
    return (EXIT_SUCCESS);
}


