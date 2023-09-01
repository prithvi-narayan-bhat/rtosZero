/**
*      @file commands.c
*      @author Prithvi Bhat
*      @brief File that handles functions relating terminal commands
*      @date 2022-12-04
**/

#include "strings.h"
#include "commands.h"
#include "gpio.h"
#include "pinMappings.h"

/**
 *      @brief Function to display the process (thread) status
 *
 **/
void ps(void)
{
    print("", "ps() invoked", CHAR);
}

/**
 *      @brief Function to display the inter-process (thread) communication status
 *
 **/
void ipcs(void)
{
    print("", "ipcs() invoked", CHAR);
}

/**
 *      @brief Function to kill the process (thread) with matching PID
 *      @param pid to match and kill
 **/
void kill(uint32_t pid)
{
    print((void *)&pid, "killed", INT);
}

/**
*      @brief Function to kill process (thread) with matching name
*      @param procName process to match and kill
**/
void Pkill(char *procName)
{
    print((void *)procName, "Process killed:", CHAR);
}

/**
*      @brief Function to toggle preemption state
*      @param state new preemption state
**/
void preempt(bool state)
{
    print((void *)&state, "Preemption state:", BOOL);
}

/**
*      @brief Function to determine scheduling order
*      @param state round robin or priority based
**/
void sched(bool state)
{
    print((void *)&state, "Priority schedule:", BOOL);
}

/**
*      @brief Function to display the PID of given process
*      @param procName whose PID to print
**/
void pidof(char *procName)
{
    uint32_t val = 10048;
    print((void *)&val, (const char *)procName, INT);
}

/**
 *      @brief Function to run selected program in the background
 *      @param procName to run in background
 **/
void run(char *procName)
{
    setPinValue(LED_R, true);
}
