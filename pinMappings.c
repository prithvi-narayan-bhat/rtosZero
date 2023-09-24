/**
*      @file pinMappings.c
*      @author Prithvi Bhat
*      @brief Functions dealing with the external hardware and gpio initialisation
**/

#include "pinMappings.h"
#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "clock.h"
#include "nvic.h"
#include "uart0.h"
#include "systemInterrups.h"

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
