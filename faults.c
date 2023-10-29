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

uint32_t pid_g;
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

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
