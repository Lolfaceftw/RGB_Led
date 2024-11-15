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


volatile unsigned int brightness = 234; // Initial of 50% brightness.

#define INIT_TOP 468 // Let x be the PER value, $100 Hz=\frac{48e6}{1024(x+1)}$

int main(int argc, char** argv) {
    Start();
    while (1) {
        Cycle_RGB();
}
        
    return (EXIT_SUCCESS);
}


