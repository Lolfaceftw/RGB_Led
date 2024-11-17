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
int normal = 1;
int pls_freeze = 1;

int main(int argc, char** argv) {

    Start();
    while (1) {
        Cycle_RGB(normal);
    }

    return (EXIT_SUCCESS);
}


