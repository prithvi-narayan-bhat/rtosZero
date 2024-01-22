/**
*      @file timer.c
*      @author Prithvi Bhat
*      @brief Timer library for Tiva C TM4C123GH6PM
*      @copyright Copyright (c) 2024
**/

#include "tm4c123gh6pm.h"
#include "timer.h"
#include <stdint.h>
#include "wait.h"

/**
*      @brief Function to initialise timer module
**/
void initTimer(void)
{
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R0;    // Enable and provide clock to the timer
    _delay_cycles(3);                               // Delay for sync

    WTIMER0_CTL_R       &= ~TIMER_CTL_TAEN;         // Disable timer before configuring
    WTIMER0_CFG_R       = TIMER_CFG_32_BIT_TIMER;   // Select 32 bit wide counter
    WTIMER0_TAMR_R      |= TIMER_TAMR_TACDIR;       // Direction = Up-counter
}