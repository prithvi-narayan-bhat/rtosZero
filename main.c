/**
 *      @file main.c
 *      @author Prithvi Bhat
 *      @brief Main driver code for the shell
 **/

#include "clock.h"
#include "gpio.h"
#include "tm4c123gh6pm.h"
#include "uart0.h"
#include "wait.h"
#include "shell.h"
#include "pinMappings.h"
#include "nvic.h"
#include "systemRegisters.h"
#include "systemInterrups.h"
#include "strings.h"


/**
 *      @brief Function to initialize all necessary hardware on the device
 **/
void initTm4c(void)
{
    initSystemClockTo40Mhz(); 		                // Initialize system clock
    initSystemInterrupts();                         // Enable system interrupts

    initUart0();                                    // Initialise UART0
    setUart0BaudRate(115200, 40e6);                 // Set UART baud rate and clock

    enablePort(PORTA);                              // Initialize clocks on PORT A
    enablePort(PORTB);                              // Initialize clocks on PORT B
    enablePort(PORTC);                              // Initialize clocks on PORT C
    enablePort(PORTF);                              // Initialize clocks on PORT F

    selectPinDigitalInput(PUB_E1);                  // Initialise pin as input
    selectPinDigitalInput(PUB_E2);                  // Initialise pin as input
    selectPinDigitalInput(PUB_E3);                  // Initialise pin as input
    selectPinDigitalInput(PUB_E4);                  // Initialise pin as input
    selectPinDigitalInput(PUB_E5);                  // Initialise pin as input
    selectPinDigitalInput(PUB_E6);                  // Initialise pin as input

    enablePinPullup(PUB_E1);                        // Enable internal pull-up
    enablePinPullup(PUB_E2);                        // Enable internal pull-up
    enablePinPullup(PUB_E3);                        // Enable internal pull-up
    enablePinPullup(PUB_E4);                        // Enable internal pull-up
    enablePinPullup(PUB_E5);                        // Enable internal pull-up
    enablePinPullup(PUB_E6);                        // Enable internal pull-up

    selectPinPushPullOutput(LED_R);                 // Initialise pin as output
    selectPinPushPullOutput(LED_G);                 // Initialise pin as output
    selectPinPushPullOutput(LED_B);                 // Initialise pin as output

    selectPinPushPullOutput(LED_EY);                // Initialise pin as output
    selectPinPushPullOutput(LED_EG);                // Initialise pin as output
    selectPinPushPullOutput(LED_ER);                // Initialise pin as output
    selectPinPushPullOutput(LED_EO);                // Initialise pin as output

    setPinValue(LED_R, 1);
    setPinValue(LED_G, 1);
    setPinValue(LED_B, 1);

    setPinValue(LED_EY, 1);
    setPinValue(LED_EG, 1);
    setPinValue(LED_ER, 1);
    setPinValue(LED_EO, 1);
}

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
*      @brief Functions to 
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
            print("", "Here", CHAR);
            FunctionPointer functionPtr = (FunctionPointer)0x40001000;  // Create a function pointer and assign the address of the customFunction
            functionPtr();                                              // Call the function through the function pointer
        }

        if(!getPinValue(PUB_E5)) setPinValue(LED_EG, 0);
        if(!getPinValue(PUB_E6)) setPinValue(LED_ER, 0);

        shell();                                            // Invoke the shell operations
    }
}
