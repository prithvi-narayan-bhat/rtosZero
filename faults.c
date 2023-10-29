// Shell functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "faults.h"

// User added
#include "strings.h"
#include "systemRegisters.h"

uint32_t pid_g;

/**
 *      @brief ISR to handle MPU faults
 *
 **/
void mpuFaultIsr(void)
{
    print((void *)&pid_g, "-> MPU fault", INT);
    uint32_t *psp;

    uint32_t flags = getFaultFlags(), faultAddr = getMemFaultAddress();

    psp = (uint32_t *)getPSP();

    print((void *)&psp, "-> PSP", HEX);
    print((void *)&flags, "-> Fault Status", HEX);
    print((void *)&faultAddr, "-> Fault Address", HEX);

    print((void *)&psp[0], "-> R0", HEX);
    print((void *)&psp[1], "-> R1", HEX);
    print((void *)&psp[2], "-> R2", HEX);
    print((void *)&psp[3], "-> R3", HEX);
    print((void *)&psp[4], "-> R12", HEX);
    print((void *)&psp[5], "-> LR", HEX);
    print((void *)&psp[6], "-> PC", HEX);
    print((void *)&psp[7], "-> XSPR", HEX);

    clearMemFaults();
    enablePendSV();
}

/**
 *      @brief ISR to handle hard faults
 *
 **/
void hardFaultIsr(void)
{
    uint32_t msp = getMSP();
    uint32_t flags = getFaultFlags();
    uint32_t psp = getPSP();

    loadPSP(psp);

    clearHardFaults();

    print((void *)&pid_g, "-> Hard fault", INT);
    print((void *)&psp, "-> PSP", HEX);
    print((void *)&msp, "-> MSP", HEX);
    print((void *)&flags, "-> Fault Flags", HEX);

    while(1);
}

/**
 *      @brief ISR to handle bus faults
 *
 **/
void busFaultIsr(void)
{
    print((void *)&pid_g, "-> Bus fault", INT);
    clearBusFault();
    while(1);
}

/**
 *      @brief ISR to handle usage faults
 *
 **/
void usageFaultIsr(void)
{
    print((void *)&pid_g, "-> Usage fault", INT);       // Debug message
    clearDivZeroFault();                                // Clear fault
    while(1);                                           // Hold infinitely
}

/**
 *      @brief Function to re-route system faults to priority defined ISRs
 **/
void initSystemInterrupts(void)
{
    enableBusFaults();     // Enable bus faults
    enableUsageFaults();   // Enable usage faults
    enableMemoryFaults();  // Enable memory usage faults
    enableDivZeroFaults(); // Enable trap on divide by zero
}
