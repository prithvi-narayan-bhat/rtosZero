/**
*      @file nvic.h
*      @author Jason Losh, Prithvi Bhat
*      @brief Nested Vector Interrupt Controller library for tm4c123gh6pm
*      @copyright Copyright (c) 2024
**/

#ifndef NVIC_H
#define NVIC_H

#include <stdint.h>
#include "tm4c123gh6pm.h"

 #define SYS_REBOOT (NVIC_APINT_R = (NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ))

#define enableBusFaults()       (NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_BUS)      // Macro to enable bus faults
#define enableUsageFaults()     (NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_USAGE)    // Macro to enable usage faults
#define enableMemoryFaults()    (NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_MEM)      // Macro to enable memory usage faults
#define enableDivZeroFaults()   (NVIC_CFG_CTRL_R     |= NVIC_CFG_CTRL_DIV0)         // Macro to enable trap on divide by zero
#define disableDivZeroFaults()  (NVIC_CFG_CTRL_R     &= ~NVIC_CFG_CTRL_DIV0)        // Macro to disable trap on divide by zero
#define clearDivZeroFault()     (NVIC_FAULT_STAT_R   |= NVIC_FAULT_STAT_DIV0)       // Clear a divide by zero fault flag
#define clearHardFaults()       (NVIC_HFAULT_STAT_R  |= NVIC_HFAULT_STAT_DBG |NVIC_HFAULT_STAT_FORCED |NVIC_HFAULT_STAT_VECT)
#define clearBusFault()         (NVIC_FAULT_STAT_R   &= ~NVIC_FAULT_STAT_IBUS)
#define enablePendSV()          (NVIC_INT_CTRL_R     |= NVIC_INT_CTRL_PEND_SV)      // Set PendSV
#define clearPendSV()           (NVIC_INT_CTRL_R     |= NVIC_INT_CTRL_UNPEND_SV)    // Set PendSV
#define getFaultFlags()         (NVIC_FAULT_STAT_R)                                 // Read fault flags
#define getPendSVFlags()        (NVIC_FAULT_STAT_R & (NVIC_FAULT_STAT_DERR | NVIC_FAULT_STAT_IERR))
#define clearPendSVFlags()      (NVIC_FAULT_STAT_R   &= ~(NVIC_FAULT_STAT_DERR | NVIC_FAULT_STAT_IERR))
#define clearMemFaults()        (NVIC_SYS_HND_CTRL_R &= ~(NVIC_SYS_HND_CTRL_MEMP))  // Clear the Mem fault pending bit
#define getMemFaultAddress()    (NVIC_FAULT_ADDR_R)                                 // Macro to read the address of the fault-causing address


void enableNvicInterrupt(uint8_t vectorNumber);
void disableNvicInterrupt(uint8_t vectorNumber);
void setNvicInterruptPriority(uint8_t vectorNumber, uint8_t priority);

#endif
