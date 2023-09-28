/**
 *      @file main.c
 *      @author Prithvi Bhat
 *      @brief Main driver code for the shell
 **/

#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "shell.h"
#include "pinMappings.h"
#include "exceptionTests.h"
#include "mpu.h"
#include "systemRegisters.h"
#include "systemInterrupts.h"

/**
 *      @brief Main, driver function
 **/
void main(void)
{
    initTm4c();
    initSystemInterrupts();                                 // Enable system interrupts

#if TEST_BACKGROUND_RULES
    setBackgroundRules();                                   // Set Background rules for undefined spaces
    allowFlashAccess();                                     // Set Flash access rules
    enableMPU();                                            // Enable MPU rules
    toggleLED(LED_B);                                       // Toggle LED to indicate working
    disablePrivilegedMode();                                // Disable privileged mode
    toggleLED(LED_R);                                       // Toggle LED to indicate working
    enablePrivilegedMode();                                 // Enable privileged mode
    toggleLED(LED_G);                                       // Toggle LED to indicate working
#endif

#if TEST_SRAM_ACCESS
    setBackgroundRules();                                   // Set Background rules for undefined spaces
    allowFlashAccess();                                     // Set Flash access rules
    setupSramAccess();                                      // Set SRAM access rules
    setSramAccessWindow((uint32_t *)0x20001200, 1024);      // Set a window to allow the RAM access
#endif

    while (1)
    {
        if(!getPinValue(PUB_E1)) busFaultTrigger();         // Trigger a bus fault
        if(!getPinValue(PUB_E2)) usageFaultTrigger();       // Trigger a usage fault
        if(!getPinValue(PUB_E3)) pendSVTrigger();           // Trigger a pendSV fault

        if(!getPinValue(PUB_E4))                            // Trigger a Memory Management Fault
        {
            FunctionPointer functionPtr = (FunctionPointer)0x40001000;  // Create a function pointer and assign the address of the customFunction
            functionPtr();                                              // Call the function through the function pointer
        }

        if(!getPinValue(PUB_E5))
        if(!getPinValue(PUB_E6))
        shell();                                            // Invoke the shell operations
    }
}
