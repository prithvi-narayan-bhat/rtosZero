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
#include "uart0.h"

#define BLUE_LED   PORTF,2 // on-board blue LED
#define RED_LED    PORTC,6 // off-board red LED
#define ORANGE_LED PORTC,7 // off-board orange LED
#define YELLOW_LED PORTC,4 // off-board yellow LED
#define GREEN_LED  PORTC,5 // off-board green LED

// User added
#define PUB_E1  PORTA,2                             // External push button
#define PUB_E2  PORTA,3                             // External push button
#define PUB_E3  PORTA,4                             // External push button
#define PUB_E4  PORTB,6                             // External push button
#define PUB_E5  PORTB,7                             // External push button
#define PUB_E6  PORTB,2                             // External push button

#define PUB_E1_PRESSED !(~buttons & 1)
#define PUB_E2_PRESSED !(~buttons & 2)
#define PUB_E3_PRESSED !(~buttons & 4)
#define PUB_E4_PRESSED !(~buttons & 8)
#define PUB_E5_PRESSED !(~buttons & 16)

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

/**
 *      @brief Function to initialise push-buttons and blue, orange, red, green, and yellow LEDs
 *
 **/
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

/**
 *      @brief Function to return a value from 0-63 indicating which of 6 PBs are pressed
 *      @return uint8_t Mask of buttons pressed
 **/
uint8_t readPbs(void)
{
    uint8_t retVal = 0;

    if      (!getPinValue(PUB_E1)) retVal = (1 << 0);    // Mask the value of the button press
    else if (!getPinValue(PUB_E2)) retVal = (1 << 1);    // Mask the value of the button press
    else if (!getPinValue(PUB_E3)) retVal = (1 << 2);    // Mask the value of the button press

    if      (!getPinValue(PUB_E4)) retVal = (1 << 3);    // Mask the value of the button press
    else if (!getPinValue(PUB_E5)) retVal = (1 << 4);    // Mask the value of the button press
    else if (!getPinValue(PUB_E6)) retVal = (1 << 5);    // Mask the value of the button press

    return retVal;
}

/**
 *      @brief One task must be ready at all times or the scheduler will fail
 *              The idle task is implemented for this purpose
 *              This task will turn on the Orange LED
 **/
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

/**
 *      @brief One task must be ready at all times or the scheduler will fail
 *              The idle task is implemented for this purpose
 *              This task will turn on the Orange LED
 **/
void idleSomeMore(void)
{
    while (true)
    {
        setPinValue(YELLOW_LED, 1);
        waitMicrosecond(1000);
        setPinValue(YELLOW_LED, 0);
        yield();
    }
}

/**
*      @brief Function to flash the Green LED at a 4Hz frequency
**/
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

/**
*      @brief Function to simply add delay to make a function lengthy
**/
void partOfLengthyFn(void)
{
    waitMicrosecond(990);                                   // Represent some lengthy operation
    yield();                                                // Give another process a chance to run
}

/**
*      @brief Function to emulate a lengthy function by setting the Red LED
**/
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

/**
 *      @brief Function to act on push button presses
 **/
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
        if (PUB_E1_PRESSED)
        {
            setPinValue(YELLOW_LED, !getPinValue(YELLOW_LED));
            setPinValue(RED_LED, 1);
            putsUart0("PUB_E1_PRESSED\r\n");
        }
        else if (PUB_E2_PRESSED)
        {
            post(flashReq);
            setPinValue(RED_LED, 0);
            putsUart0("PUB_E2_PRESSED\r\n");
        }
        else if (PUB_E3_PRESSED)
        {
            restartThread(flash4Hz);
            putsUart0("PUB_E3_PRESSED\r\n");
        }
        else if (PUB_E4_PRESSED)
        {
            stopThread(flash4Hz);
            putsUart0("PUB_E4_PRESSED\r\n");
        }
        else if (PUB_E5_PRESSED)
        {
            setThreadPriority(lengthyFn, 4);
            putsUart0("PUB_E5_PRESSED\r\n");
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

/**
*      @brief A function with the highest priority that toggles the blue LED
**/
void important(void)
{
    while(true)
    {
        lock(resource);
        setPinValue(BLUE_LED, 1);
        sleep(1000);
        setPinValue(BLUE_LED, 0);
        sleep(1000);
        unlock(resource);
    }
}
