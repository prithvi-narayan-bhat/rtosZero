/**
*      @file exceptionTests.c
*      @author Prithvi Bhat
*      @brief Function to test the various exceptions and fault handlers
**/

#include <inttypes.h>
#include "systemInterrups.h"
#include "tm4c123gh6pm.h"

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

void customFunction()                               // Define a function at the desired address (0x00040000)
{
    uint8_t a = 5, b = 10;
    a = a + b;
}
