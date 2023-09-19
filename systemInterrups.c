#include "strings.h"
#include "systemRegisters.h"
#include "tm4c123gh6pm.h"

#define enableBusFaults()       (NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_BUS)      // Function to enable bus faults
#define enableUsageFaults()     (NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_USAGE)    // Function to enable usage faults
#define enableMemoryFaults()    (NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_MEM)      // Function to enable memory usage faults

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
}

/**
 *      @brief ISR to handle bus faults
 *
 **/
void busFaultISR(void)
{
    print((void *)&pid_g, "-> Bus fault", INT);
}

/**
 *      @brief ISR to handle usage faults
 *
 **/
void usageFaultISR(void)
{
    print((void *)&pid_g, "-> Usage fault", INT);
}

/**
 *      @brief ISR to handle MPU faults
 *
 **/
void mpuFaultISR(void)
{
    print((void *)&pid_g, "-> MPU fault", INT);
}

/**
 *      @brief ISR to handle hard faults
 *
 **/
void hardFaultISR(void)
{
    uint32_t psp = 0, msp = 0, flags = 0, address = 0xE000ED2C;
    msp = getMSP();
    flags = getFaultFlags(&address);
    switchToPSP(0x00000002);
    psp = getPSP();

    print((void *)&pid_g, "Hard fault", INT);
    print((void *)&psp, "-> PSP", HEX);
    print((void *)&msp, "-> MSP", HEX);
    print((void *)&flags, "-> Hex Fault Status registers", HEX);

    while(1);
}

/**
 *      @brief ISR to handle pendSV faults
 *
 **/
void pendSvISR(void)
{
    print("", "PendSV fault in pid", INT);
}