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
#include "memory.h"
#include "strings.h"

/**
 *      @brief Main, driver function
 **/
void main(void)
{
    initTm4c();
    initSystemInterrupts();                                 // Enable system interrupts

    uint32_t *ptr = (uint32_t *)malloc_from_heap(2048);
    uint32_t *ptr2 = (uint32_t *)malloc_from_heap(1024);
    uint32_t *ptr3 = (uint32_t *)malloc_from_heap(4096);
    uint32_t *ptr4 = (uint32_t *)malloc_from_heap(3072);
    uint32_t *ptr5 = (uint32_t *)malloc_from_heap(1536);
    uint32_t *ptr6 = (uint32_t *)malloc_from_heap(1536);
    uint32_t *ptr7 = (uint32_t *)malloc_from_heap(1536);

    if (ptr7 != NULL)
    {
        print((void *)&ptr, "->Address", HEX);
        print((void *)&ptr2, "->Address", HEX);
        print((void *)&ptr3, "->Address", HEX);
        print((void *)&ptr4, "->Address", HEX);
        print((void *)&ptr5, "->Address", HEX);
        print((void *)&ptr6, "->Address", HEX);
        print((void *)&ptr7, "->Address", HEX);
    }
    else
    {
        print("", "Memory allocation failed", CHAR);
    }

    free((void *)ptr);

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

    setBackgroundRules();                                   // Set Background rules for undefined spaces
    allowFlashAccess();                                     // Set Flash access rules
    setupSramAccess();                                      // Set SRAM access rules
    setSramAccessWindow((uint32_t *)ptr5, 1536);            // Set a window to allow the RAM access

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
