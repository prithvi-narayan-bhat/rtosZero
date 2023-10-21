// Tasks
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "wait.h"
#include "kernel.h"
#include "tasks.h"

#define BLUE_LED   PORTF,2 // on-board blue LED
#define RED_LED    PORTC,6 // off-board red LED
#define ORANGE_LED PORTA,2 // off-board orange LED
#define YELLOW_LED PORTC,4 // off-board yellow LED
#define GREEN_LED  PORTC,7 // off-board green LED

// User added
#define PUB_E1  PORTA,2                             // External push button
#define PUB_E2  PORTA,3                             // External push button
#define PUB_E3  PORTA,4                             // External push button
#define PUB_E4  PORTB,6                             // External push button
#define PUB_E5  PORTB,7                             // External push button
#define PUB_E6  PORTB,2                             // External push button
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
// REQUIRED: Add initialization for blue, orange, red, green, and yellow LEDs
//           Add initialization for 6 pushbuttons
void initHw(void)
{
    // Setup LEDs and pushbuttons
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

    selectPinPushPullOutput(BLUE_LED);              // Initialise pin as output

    selectPinPushPullOutput(YELLOW_LED);            // Initialise pin as output
    selectPinPushPullOutput(GREEN_LED);             // Initialise pin as output
    selectPinPushPullOutput(RED_LED);               // Initialise pin as output
    selectPinPushPullOutput(ORANGE_LED);            // Initialise pin as output

    // Power-up flash
    setPinValue(GREEN_LED, 1);
    waitMicrosecond(250000);
    setPinValue(GREEN_LED, 0);
    waitMicrosecond(250000);
}

// REQUIRED: add code to return a value from 0-63 indicating which of 6 PBs are pressed
uint8_t readPbs(void)
{
    uint8_t retVal = 0;

    if(!getPinValue(PUB_E1)) retVal |= (1 << 0);
    if(!getPinValue(PUB_E2)) retVal |= (1 << 1);
    if(!getPinValue(PUB_E3)) retVal |= (1 << 2);
    if(!getPinValue(PUB_E4)) retVal |= (1 << 3);
    if(!getPinValue(PUB_E5)) retVal |= (1 << 4);
    if(!getPinValue(PUB_E6)) retVal |= (1 << 5);

    return retVal;
}

// one task must be ready at all times or the scheduler will fail
// the idle task is implemented for this purpose
void idle(void)
{
    while(true)
    {
        setPinValue(ORANGE_LED, 1);
        waitMicrosecond(1000);
        setPinValue(ORANGE_LED, 0);
        yield();
    }
}

void flash4Hz(void)
{
    while(true)
    {
        setPinValue(GREEN_LED, !getPinValue(GREEN_LED));
        sleep(125);
    }
}

void oneshot(void)
{
    while(true)
    {
        wait(flashReq);
        setPinValue(YELLOW_LED, 1);
        sleep(1000);
        setPinValue(YELLOW_LED, 0);
    }
}

void partOfLengthyFn(void)
{
    // represent some lengthy operation
    waitMicrosecond(990);
    // give another process a chance to run
    yield();
}

void lengthyFn(void)
{
    uint16_t i;
    while(true)
    {
        lock(resource);
        for (i = 0; i < 5000; i++)
        {
            partOfLengthyFn();
        }
        setPinValue(RED_LED, !getPinValue(RED_LED));
        unlock(resource);
    }
}

void readKeys(void)
{
    uint8_t buttons;
    while(true)
    {
        wait(keyReleased);
        buttons = 0;
        while (buttons == 0)
        {
            buttons = readPbs();
            yield();
        }
        post(keyPressed);
        if ((buttons & 1) != 0)
        {
            setPinValue(YELLOW_LED, !getPinValue(YELLOW_LED));
            setPinValue(RED_LED, 1);
        }
        if ((buttons & 2) != 0)
        {
            post(flashReq);
            setPinValue(RED_LED, 0);
        }
        if ((buttons & 4) != 0)
        {
            restartThread(flash4Hz);
        }
        if ((buttons & 8) != 0)
        {
            stopThread(flash4Hz);
        }
        if ((buttons & 16) != 0)
        {
            setThreadPriority(lengthyFn, 4);
        }
        yield();
    }
}

void debounce(void)
{
    uint8_t count;
    while(true)
    {
        wait(keyPressed);
        count = 10;
        while (count != 0)
        {
            sleep(10);
            if (readPbs() == 0)
                count--;
            else
                count = 10;
        }
        post(keyReleased);
    }
}

void uncooperative(void)
{
    while(true)
    {
        while (readPbs() == 8)
        {
        }
        yield();
    }
}

void errant(void)
{
    uint32_t* p = (uint32_t*)0x20000000;
    while(true)
    {
        while (readPbs() == 32)
        {
            *p = 0;
        }
        yield();
    }
}

void important(void)
{
    while(true)
    {
        lock(resource);
        setPinValue(BLUE_LED, 1);
        sleep(1000);
        setPinValue(BLUE_LED, 0);
        unlock(resource);
    }
}
