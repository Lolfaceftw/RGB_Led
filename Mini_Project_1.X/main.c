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
float multiplier = 0.5f;
int delay = 400;
int normal = 1;

int read_count(){
    // Allow read access of COUNT register
    // Return back the counter value
    TC0_REGS -> COUNT16.TC_CTRLBSET = (0x4 << 5);
    return TC0_REGS -> COUNT16.TC_COUNT; // 39.8.13
    
}

int main(int argc, char** argv) {

    Start();
    while (1) {
        Cycle_RGB(multiplier, delay, normal);
}
       
    return (EXIT_SUCCESS);
}


