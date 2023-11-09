/**
*      @file strings.h
*      @author Prithvi Bhat
*      @brief Header file for string parsing and allied operational commands
**/
#ifndef STRINGS_H
#define STRINGS_H

#include "uart0.h"
#include <inttypes.h>

#define MAX_STRING_LENGTH   80          // Maximum number of characters in input string
#define MAX_FIELDS          10          // Maximum number of acceptable arguments

typedef struct
{
    char inputString[MAX_STRING_LENGTH];
    char type[MAX_FIELDS];
    uint8_t position[MAX_FIELDS];
    uint8_t count;
} shellData_t;

typedef enum
{
    INT     = 0,
    BOOL    = 1,
    CHAR    = 2,
    HEX     = 3,
} argType_t;

// Function prototypes
// String functions
char *itoa(int32_t number, char *string);
void toLower(char *s1);
bool toBool(char *string);
int strcmp(const char *s1, const char *s2);
char *strcat(char *s1, const char *s2);
void strcpy(char *s1, const char *s2);
void print(void *s1, const char *s2, argType_t type);
char *htoa(uint32_t hexValue, char *string);
uint32_t asciiToHex(const char *s);

// Application functions
void getInputString(shellData_t *shellData);
void parseInputString(shellData_t *shellData);
char *getFieldString(shellData_t *shellData, uint8_t fieldNumber);
int32_t getFieldInteger(shellData_t *shellData, uint8_t fieldNumber);
bool isCommand(shellData_t *userData, const char *cmp, uint8_t arg_count);

#endif
