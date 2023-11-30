/**
*      @file commands.c
*      @author Prithvi Bhat
*      @brief File that handles functions relating terminal commands
*      @date 2022-12-04
**/

#ifndef COMMANDS_H
#define COMMANDS_H

#include <inttypes.h>
#include <stdbool.h>

void ps(void *ptr);             // Function to display the process (thread) status
void ipcs(void *, void *);      // Function to display the inter-process (thread) communication status
void kill(uint32_t pid);        // Function to kill the process (thread) with matching PID
void Pkill(char *procName);     // Function to kill process (thread) with matching name
void preempt(bool state);       // Function to toggle preemption state
void priority(bool state);      // Function to determine scheduling order
void pidof(char *, void *);     // Function to display the PID of given process
void run(char *procName);       // Function to run selected program in the background
void reboot(void);              // Function to reset the system
void inheritance(bool state);   // Function to change priority inheritance mode

#endif
