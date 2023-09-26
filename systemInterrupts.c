#include "strings.h"
#include "systemRegisters.h"
#include "tm4c123gh6pm.h"
#include "systemInterrupts.h"
// Todo Define extern
uint32_t pid_g = 1034;

/**
*      @brief Function to re-route system faults to priority defined ISRs
**/
void initSystemInterrupts(void)
{
    enableBusFaults();          // Enable bus faults
    enableUsageFaults();        // Enable usage faults
    enableMemoryFaults();       // Enable memory usage faults
    enableDivZeroFaults();      // Enable trap on divide by zero
}

/**
 *      @brief ISR to handle bus faults
 *
 **/
void busFaultISR(void)
{
    print((void *)&pid_g, "-> Bus fault", INT);
    clearBusFault();
}

/**
 *      @brief ISR to handle usage faults
 *
 **/
void usageFaultISR(void)
{
    print((void *)&pid_g, "-> Usage fault", INT);       // Debug message
    clearDivZeroFault();                                // Clear fault
    while(1);                                           // Hold infinitely
}

/**
 *      @brief ISR to handle MPU faults
 *
 **/
void mpuFaultISR(void)
{
    print((void *)&pid_g, "-> MPU fault", INT);
    uint32_t *msp = getMSP();

    uint32_t mspPtr = getMSP(), flags = getFaultFlags(), faultAddr = getMemFaultAddress();

    // switchToPSP(0x00000002);
    // psp = getPSP();

    print((void *)&mspPtr, "-> MSP", HEX);
    print((void *)&flags, "-> Fault Status", HEX);
    print((void *)&faultAddr, "-> Fault Address", HEX);

    print((void *)&msp[0], "-> R0", HEX);
    print((void *)&msp[1], "-> R1", HEX);
    print((void *)&msp[2], "-> R2", HEX);
    print((void *)&msp[3], "-> R3", HEX);
    print((void *)&msp[4], "-> R12", HEX);
    print((void *)&msp[5], "-> LR", HEX);
    print((void *)&msp[6], "-> PC", HEX);
    print((void *)&msp[7], "-> XSPR", HEX);
    clearMemFaults();
    enablePendSV();
}

/**
 *      @brief ISR to handle hard faults
 *
 **/
void hardFaultISR(void)
{
    uint32_t psp = 0, msp = 0, flags = 0;
    msp = getMSP();
    flags = getFaultFlags();
    // switchToPSP(0x00000002);
    psp = getPSP();

    clearHardFaults();

    print((void *)&pid_g, "-> Hard fault", INT);
    print((void *)&psp, "-> PSP", HEX);
    print((void *)&msp, "-> MSP", HEX);
    print((void *)&flags, "-> Fault Flags", HEX);

    while(1);
}

/**
 *      @brief ISR to handle pendSV faults
 *
 **/
void pendSvISR(void)
{
    print("", "PendSV fault in pid", CHAR);
    if (getPendSVFlags())
    {
        print("", "Called from pendSV", CHAR);
        clearPendSVFlags();
        while(1);
    }
}
