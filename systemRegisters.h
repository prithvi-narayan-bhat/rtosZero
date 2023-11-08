#ifndef SYSTEM_REGISTERS_H
#define SYSTEM_REGISTERS_H

#include <inttypes.h>

extern uint32_t getPSP(void);                       // Function to read and return the value of the PSP register
extern uint32_t getMSP(void);                       // Function to read and return the value of the MSP register
extern uint32_t disablePrivilegedMode(void);        // Function to disable Privileged execution mode
extern uint32_t enablePrivilegedMode(void);         // Function to enable Privileged execution mode
extern void loadPSP(uint32_t stackAddr);            // Function to load the stack pointer and set the ASP bit in the CONTROL register
extern void setASP(void);                           // Function to set the ASP bit in the CONTROL register
extern void stageMethod(uint32_t stackAddr);        // Function to load a value to the stack pointer
extern uint32_t getValue(uint32_t);                 // Get a value
extern uint32_t getSvcPriority(void);               // Return the SVC priority
extern uint32_t getTicks(void);                     // Return the sleep Tick value

#endif
