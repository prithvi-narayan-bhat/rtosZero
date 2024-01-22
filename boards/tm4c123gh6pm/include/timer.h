/**
*      @file timer.h
*      @author Prithvi Bhat
*      @brief Timer library for TM4C123GH6PM
*      @copyright Copyright (c) 2024
**/
#ifndef TIMER_H
#define TIMER_H

#define STOP_TIMER      (WTIMER0_CTL_R &= ~TIMER_CTL_TAEN)
#define RESET_TIMER     (WTIMER0_TAV_R = 0)
#define RESTART_TIMER   (WTIMER0_CTL_R |= TIMER_CTL_TAEN)

void initTimer(void);

#endif