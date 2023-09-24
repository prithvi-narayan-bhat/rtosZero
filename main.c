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
