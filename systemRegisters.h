#ifndef SYSTEM_REGISTERS_H
#define SYSTEM_REGISTERS_H

#include <inttypes.h>

extern void switchToPSP(uint32_t value);            // Function to switch operation control to PSP from MSP
extern uint32_t getPSP(void);                       // Function to read and return the value of the PSP register
extern uint32_t getMSP(void);                       // Function to read and return the value of the MSP register
extern uint32_t getFaultFlags(uint32_t *regAddr);   // Function to read the fault flags
extern uint32_t disablePrivilegedMode(void);        // Function to disable Privileged execution mode
extern uint32_t enablePrivilegedMode(void);         // Function to enable Privileged execution mode
void loadPSP(uint32_t stackAddr);                   // Function to load a value to the stack pointer

#endif
