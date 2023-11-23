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
    __asm(" SVC #0x09");                                    // Trigger a Service call
}

/**
 *      @brief Function to display the inter-process (thread) communication status
 *
 **/
void ipcs(void)
{
    __asm(" SVC #0x16");                                    // Trigger a Service call
}

/**
 *      @brief Function to stop a thread/task
 *      @param pid of function to be killed
 **/
void kill(uint32_t pid)
{
    __asm(" SVC #0x06");                                    // Trigger a Service call
}

/**
 *      @brief Function to reset the system
 **/
void reboot(void)
{
    __asm(" SVC #0x08");                                    // Trigger a Service call
}

/**
*      @brief Function to kill process (thread) with matching name
*      @param procName process to match and kill
**/
void Pkill(char *procName)
{
    __asm(" SVC #0x14");                                    // Trigger a Service call
}

/**
*      @brief Function to toggle preemption state
*      @param state new preemption state
**/
void preempt(bool state)
{
    __asm(" SVC #0x12");                                    // Trigger a Service call
}

/**
*      @brief Function to determine scheduling order
*      @param state round robin or priority based
**/
void priority(bool state)
{
    __asm(" SVC #0x11");                                    // Trigger a Service call
}

/**
*      @brief Function to display the PID of given process
*      @param procName whose PID to print
**/
void pidof(char *procName)
{
    __asm(" SVC #0x13");                                    // Trigger a Service call
}

/**
 *      @brief Function to run selected program in the background
 *      @param procName to run in background
 **/
void run(char *procName)
{
    __asm(" SVC #0x15");                                    // Trigger a Service call
}

/**
 *      @brief Function to toggle priority inheritance state
 *      @param state new priority inheritance state
 **/
void inheritance(bool state)
{
    __asm(" SVC #0x18");                                    // Trigger a Service call
}