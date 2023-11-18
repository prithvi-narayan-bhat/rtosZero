// RTOS Framework - Fall 2023
// J Losh

// Student Name: Prithvi Bhat
// TO DO: Add your name(s) on this line.
//        Do not include your ID number(s) in the file.

// Please do not change any function name in this code or the thread priorities

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// 6 Pushbuttons and 5 LEDs, UART
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1
// Memory Protection Unit (MPU):
//   Region to control access to flash, peripherals, and bitbanded areas
//   4 or more regions to allow SRAM access (RW or none for task)

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include "tm4c123gh6pm.h"
#include "clock.h"
#include "gpio.h"
#include "uart0.h"
#include "wait.h"
#include "mm.h"
#include "kernel.h"
#include "faults.h"
#include "tasks.h"
#include "shell.h"

/**
 *      @brief Placeholder function to initialise everything on system start
 *
 **/
void main(void)
{
    bool ok;

    initSystemClockTo40Mhz();                                   // Initialize System clock
    initHw();                                                   // Initialize LEDs and buttons
    initUart0();                                                // Initialise console UART
    initSystemInterrupts();                                     // Initialise interrupts
    initMpu();                                                  // Initialise MPU rules and regions
    initRtos();                                                 // Initialise the RTOS

    setUart0BaudRate(115200, 40e6);                             // Setup UART0 baud rate

    // Initialize mutexes and semaphores
    initMutex(resource);
    initSemaphore(keyPressed, 1);
    initSemaphore(keyReleased, 0);
    initSemaphore(flashReq, 5);

    ok = createThread(idle, "Idle", 7, 512);                    // Add an Idle process at lowest priority
    // ok &= createThread(idleSomeMore, "idleSomeMore", 7, 512);   // Add an Idle process at lowest priority

    // Add other processes
    ok &= createThread(lengthyFn, "LengthyFn", 6, 1024);        // Add a lengthy process at a relatively high priority
    ok &= createThread(flash4Hz, "Flash4Hz", 4, 1024);          // Flash LED at 4Hz frequency
    ok &= createThread(oneshot, "OneShot", 2, 1024);
    ok &= createThread(readKeys, "ReadKeys", 6, 1024);
    ok &= createThread(debounce, "Debounce", 6, 1024);
    ok &= createThread(important, "Important", 0, 1024);        // Toggle LED at the highest priority
    // ok &= createThread(uncooperative, "Uncoop", 6, 1024);
    // ok &= createThread(errant, "Errant", 6, 1024);
    // ok &= createThread(shell, "Shell", 6, 4096);

    if(ok)      startRtos();                                    // Start up RTOS (never returns)
    else        while(true);
}
