#ifndef PIN_MAPPINGS_H
#define PIN_MAPPINGS_H

#define LED_R   PORTF,1     // Internal LED pin
#define LED_B   PORTF,2     // Internal LED pin
#define LED_G   PORTF,3     // Internal LED pin

#define LED_EY  PORTC,4     // External LED pin
#define LED_EG  PORTC,5     // External LED pin
#define LED_ER  PORTC,6     // External LED pin
#define LED_EO  PORTC,7     // External LED pin

#define PUB_E1  PORTA,2     // External push button
#define PUB_E2  PORTA,3     // External push button
#define PUB_E3  PORTA,4     // External push button
#define PUB_E4  PORTB,6     // External push button
#define PUB_E5  PORTB,7     // External push button
#define PUB_E6  PORTB,2     // External push button

void initTm4c(void);        // Function to enable pins and system clock

#endif
