/**
 *      @file clock.c
 *      @author Prithvi Bhat
 *      @brief Clock and system tick library for Tiva C TM4C123GH6PM
 *      @copyright Copyright (c) 2024
 **/

#ifndef CLOCK_H
#define CLOCK_H

void initSystemClockTo40Mhz(void);
void initSysTick(void);

#endif
