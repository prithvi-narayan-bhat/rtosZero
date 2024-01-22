/**
 *      @file clock.c
 *      @author Prithvi Bhat
 *      @brief Clock and system tick library for Tiva C TM4C123GH6PM
 *              Hardware configuration: 16 MHz external crystal oscillator
 *      @copyright Copyright (c) 2024
 **/

#include <stdint.h>
#include "clock.h"
#include "tm4c123gh6pm.h"

/**
 *      @brief Function to initialize system clock to 40 MHz using PLL and 16 MHz crystal oscillator
 *
 **/
void initSystemClockTo40Mhz(void)
{
    SYSCTL_RCC_R  = SYSCTL_RCC_XTAL_16MHZ;          // COnfigure 16MHz Crystal Oscillator
    SYSCTL_RCC_R |= SYSCTL_RCC_OSCSRC_MAIN;         // Enable PLL
    SYSCTL_RCC_R |= SYSCTL_RCC_USESYSDIV;           // Use clock divider
    SYSCTL_RCC_R |= (4 << SYSCTL_RCC_SYSDIV_S);     // Division factor
}

/**
 *      @brief Initialisation for sysTicks
 **/
void initSysTick(void)
{
    NVIC_ST_RELOAD_R = 39999;                       // sys_clock * 1 * 10^-3 for a 1ms tick
    NVIC_ST_CURRENT_R = NVIC_ST_CURRENT_M;          // Clear current value by writing any value

    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_CLK_SRC;         // Enable system clock source for Systick operation
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_INTEN;           // Enable systick interrupts
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE;          // Start systick
}