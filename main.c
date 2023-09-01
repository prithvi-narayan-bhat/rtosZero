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

#define IS_COMMAND(string, count)       if(isCommand(&shellData, string, count))
#define RESET                           (NVIC_APINT_R = (NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ))
#define ASSERT(value)                   if(value >= 0)


/**
 *      @brief Function to initialize all necessary hardware on the device
 **/
void init_TM4C_hardware(void)
{
    initSystemClockTo40Mhz(); 		                // Initialize system clock

    enablePort(PORTF);                              // Initialize clocks on PORT F
    selectPinPushPullOutput(LED_R);                 // Initialise red LED gpio as output
    selectPinPushPullOutput(LED_G);                 // Initialise green LED gpio as output
    selectPinPushPullOutput(LED_B);                 // Initialise blue LED gpio as output

    setPinValue(LED_R, 1);
    setPinValue(LED_G, 1);
    setPinValue(LED_B, 1);

    waitMicrosecond(20000);

    setPinValue(LED_R, 0);
    setPinValue(LED_G, 0);
    setPinValue(LED_B, 0);

    initUart0();                                    // Initialise UART0
    setUart0BaudRate(115200, 40e6);                 // Set UART baud rate and clock
}

/**
 *      @brief Main, driver function
 **/
void main(void)
{
    init_TM4C_hardware();

    shellData_t shellData;
    while (1)
    {
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
    }
}
