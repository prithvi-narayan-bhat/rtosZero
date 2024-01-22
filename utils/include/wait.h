/**
*      @file wait.h
*      @author Jason Losh, Prithvi Bhat
*      @brief Wait and delay library
*      @copyright Copyright (c) 2024
**/

#ifndef WAIT_H
#define WAIT_H

#include <stdint.h>

void waitMicrosecond(uint32_t us);
void _delay_cycles(uint32_t cycles);

#endif
