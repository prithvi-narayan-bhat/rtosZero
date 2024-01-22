/**
*      @file faults.h
*      @author Prithvi Bhat
*      @brief Fault handlers
*      @copyright Copyright (c) 2024
**/

#ifndef FAULTS_H
#define FAULTS_H

void mpuFaultIsr(void);
void hardFaultIsr(void);
void busFaultIsr(void);
void usageFaultIsr(void);
void initSystemInterrupts(void);

#endif
