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
#include "strings.h"
#include "commands.h"
#include "pinMappings.h"
#include "nvic.h"

#define IS_COMMAND(string, count)       if(isCommand(&shellData, string, count))
#define RESET                           (NVIC_APINT_R = (NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ))
#define ASSERT(value)                   if(value >= 0)


/**
 *      @brief Function to initialize all necessary hardware on the device
 **/
void initTm4c(void)
{
    initSystemClockTo40Mhz(); 		                // Initialize system clock

    enablePort(PORTA);                              // Initialize clocks on PORT A
    enablePort(PORTB);                              // Initialize clocks on PORT B
    enablePort(PORTC);                              // Initialize clocks on PORT B
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

    initUart0();                                    // Initialise UART0
    setUart0BaudRate(115200, 40e6);                 // Set UART baud rate and clock
}

/**
 *      @brief Main, driver function
 **/
void main(void)
{
    initTm4c();

    shellData_t shellData;
    while (1)
    {
        if(!getPinValue(PUB_E1)) setPinValue(LED_R, 0);
        if(!getPinValue(PUB_E2)) setPinValue(LED_G, 0);
        if(!getPinValue(PUB_E3)) setPinValue(LED_EO, 0);
        if(!getPinValue(PUB_E4)) setPinValue(LED_EY, 0);
        if(!getPinValue(PUB_E5)) setPinValue(LED_EG, 0);
        if(!getPinValue(PUB_E6)) setPinValue(LED_ER, 0);
#if 0
        getInputString(&shellData);         // Read user input
        parseInputString(&shellData);       // Parse user input

        IS_COMMAND("ps", 1)                 // Compare and act on user input
        {
            ps();                           // Invoke function
            continue;
        }

        IS_COMMAND("reboot", 1)
        {
            RESET;                          // Reset System
            continue;
        }

        IS_COMMAND("ipcs", 1)
        {
            ipcs();                         // Invoke function
            continue;
        }

        IS_COMMAND("kill", 2)
        {
            uint32_t pid = (uint32_t)getFieldInteger(&shellData, 1);
            kill(pid);                      // Invoke function
            continue;
        }

        IS_COMMAND("pkill", 2)              // Invoke function
        {
            char *procName = getFieldString(&shellData, 1);
            toLower(procName);
            Pkill(procName);
            continue;
        }

        IS_COMMAND("preempt", 2)
        {
            char *preemptionState = getFieldString(&shellData, 1);
            preempt(toBool(preemptionState));
            continue;
        }

        IS_COMMAND("sched", 2)
        {
            char *scheduleState = getFieldString(&shellData, 1);
            sched(toBool(scheduleState));
            continue;
        }

        IS_COMMAND("pidof", 2)
        {
            char *procName = getFieldString(&shellData, 1);
            pidof(procName);
            continue;
        }

        IS_COMMAND("run", 2)
        {
            char *procName = getFieldString(&shellData, 1);
            run(procName);
            continue;
        }

        print((void *)"", "Invalid input", CHAR);
#endif
    }
}
