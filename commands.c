/**
*      @file commands.c
*      @author Prithvi Bhat
*      @brief File that handles functions relating terminal commands
*      @date 2022-12-04
**/

#include "strings.h"
#include "commands.h"
#include "tm4c123gh6pm.h"
#include "wait.h"
#include "gpio.h"
#include "pinMappings.h"

/**
 *      @brief Function to display the process (thread) status
 *
 **/
void ps(void)
{
    putsUart0("ps() invoked\r\n");
}

/**
 *      @brief Function to display the inter-process (thread) communication status
 *
 **/
void ipcs(void)
{
    putsUart0("ipcs() invoked\r\n");
}

/**
 *      @brief Function to kill the process (thread) with matching PID
 *      @param pid to match and kill
 **/
void kill(uint32_t pid)
{
    putsUart0("PID killed\r\n");
}

/**
*      @brief Function to kill process (thread) with matching name
*      @param procName process to match and kill
**/
void Pkill(char *procName)
{
    putsUart0("procName killed\r\n");
}

/**
*      @brief Function to toggle preemption state
*      @param state new preemption state
**/
void preempt(bool state)
{
    putsUart0("preempt() invoked\r\n");
}

/**
*      @brief Function to determine scheduling order
*      @param state round robin or priority based
**/
void sched(bool state)
{
    if (state)  putsUart0("sched prio\r\n");
    else        putsUart0("sched rr\r\n");
}

/**
*      @brief Function to display the PID of given process
*      @param procName whose PID to print
**/
void pidof(char *procName)
{
    putsUart0(("pidof() invoked\r\n"));
}

/**
 *      @brief Function to run selected program in the background
 *      @param procName to run in background
 **/
void run(char *procName)
{
    setPinValue(LED_R, true);
}
