/**
 *      @file main.c
 *      @author Prithvi Bhat
 *      @brief Main driver code for the shell
 **/

#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "wait.h"
#include "shell.h"
#include "pinMappings.h"
#include "systemInterrups.h"

/**
*      @brief Function to cause a bus fault
**/
void busFaultTrigger(void)
{
    uint32_t *tester = (uint32_t *)0xFFFFFFFF;      // Access out-of-bounds pointer
    *tester = 10;                                   // Trigger a bus fault
}

/**
 *      @brief Function to cause a usage fault
 **/
void usageFaultTrigger(void)
{
    uint8_t div = 0;
    uint8_t res = 10 / div;                         // Trigger a divide by zero fault
}

/**
*      @brief Function to cause a usage fault
**/
void pendSVTrigger(void)
{
    enablePendSV();                                 // Set PendSV
}

/**
*      @brief Functions to cause a memory management fault
**/
typedef void (*FunctionPointer)(void);              // Define a function pointer type for a function that takes no arguments and returns void

void customFunction()                               // Define a function at the desired address (0x00040000)
{
    uint8_t a = 5, b = 10;
    a = a + b;
}

/**
 *      @brief Main, driver function
 **/
void main(void)
{
    initTm4c();

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

        if(!getPinValue(PUB_E5)) setPinValue(LED_EG, 0);
        if(!getPinValue(PUB_E6)) setPinValue(LED_ER, 0);

        shell();                                            // Invoke the shell operations
    }
}
