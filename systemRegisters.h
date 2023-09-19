#ifndef SYSTEM_REGISTERS_H
#define SYSTEM_REGISTERS_H

extern void switchToPSP(uint32_t value);            // Function to switch operation control to PSP from MSP
extern uint32_t getPSP(void);                       // Function to read and return the value of the PSP register
extern uint32_t getMSP(void);                       // Function to read and return the value of the MSP register
extern uint32_t getFaultFlags(uint32_t *regAddr);   // Function to read the fault flags

#endif
