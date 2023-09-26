#ifndef EXCEPTION_TESTS_H
#define EXCEPTION_TESTS_H

typedef void (*FunctionPointer)(void); // Define a function pointer type for a function that takes no arguments and returns void

void busFaultTrigger(void);
void usageFaultTrigger(void);
void pendSVTrigger(void);
void customFunction();
void toggleLED(PORT port, uint8_t pin);

#endif