/**
*      @file strings.c
*      @author Prithvi Bhat
*      @brief Source file for string parsing and allied operational commands
**/

#include <stdlib.h>
#include "uart0.h"
#include "strings.h"
#include <inttypes.h>

#define ASCII_BACKSPACE         8
#define ASCII_DELETE            127
#define ASCII_CARRIAGE_RETURN   13
#define ASCII_UPPER_ALPHABET_A  65
#define ASCII_UPPER_ALPHABET_Z  90
#define ASCII_LOWER_ALPHABET_A  97
#define ASCII_LOWER_ALPHABET_Z  122

#define RESET(c)                (c = 0)                                                 // Macro to reassign value to 0
#define ASSERT_CLEAR(c)         ((c == ASCII_BACKSPACE || c == ASCII_DELETE) ? 1 : 0)   // Macro to validate character is ASCII backspace or ASCII delete
#define ASSERT_PRINTABLE(c)     ((c >= 32 && c <= 126) ? 1 : 0)                         // Macro to validate if character is ASCII printable
#define ASSERT_EOL(c)           ((c == ASCII_CARRIAGE_RETURN) ? 1 : 0)                  // Macro to validate if character is ASCII carriage return
#define ASSERT_BUFFER_FULL(c)   ((c >= (MAX_STRING_LENGTH - 1)) ? 1 : 0)                // Macro to validate if input string buffer is full
#define ASSERT_NUMBER(c)        ((c >= 48) && (c <= 57) ? 1 : 0)                        // Macro to validate if character is number


// Macro to validate if character is an alphabet
#define ASSERT_ALPHABET(c)      (                                                                           \
                                    (                                                                       \
                                        ((c >= ASCII_UPPER_ALPHABET_A) && (c <= ASCII_UPPER_ALPHABET_Z))    \
                                        || ((c >= ASCII_LOWER_ALPHABET_A) && (c <= ASCII_LOWER_ALPHABET_Z)) \
                                    )                                                                       \
                                    ? 1 : 0                                                                 \
                                )

// Macro to convert input character to lower case
char _toLower(c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c + ('a' - 'A');
    }
    return c;
}

/**
*      @brief Function to convert a given string to lower case
*      @param string to be converted
**/
char *toLower(char *string)
{
    while (*string)
    {
        *string = _toLower(*string);
        string++;
    }
    return string;
}

/**
*      @brief Function to convert a given number to string
*      @param string Character pointer to hold converted integer
*      @param number to covert to string
*      @return char* pointer for recursive calls
**/
static char *itoa_helper(char *string, int32_t number)
{
    if (number <= -10)   string = itoa_helper(string, number / 10);

    *string++ = '0' - number % 10;
    return string;
}

/**
*      @brief Function to convert integer to string
*      @param string destination to store converted string
*      @param number to convert
*      @return char*
**/
char *itoa(char *string, int32_t number)
{
    char *s = string;
    if (number < 0)  *s++ = '-';
    else        number = -number;                                                       // Append negative sign to start of number

    *itoa_helper(s, number) = '\0';                                                     // Append NULL character to indicate end of string
    return string;
}

/**
*      @brief Function to compare strings
*      @param s1 string 1
*      @param s2 string 2
*      @return int result of comparison
**/
int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}


/**
*      @brief Function to get user input over UART
*      @param userData Pointer to userData structure
**/
void getInputString(shellData_t *userData)
{
    uint8_t character;
    uint16_t character_count;

    for (character_count = 0; character_count < MAX_STRING_LENGTH; character_count++)
    {
        character = getcUart0();                                                        // Read characters from user terminal
        if (ASSERT_CLEAR(character) && character_count > 0)                             // Validate
        {
            character_count--;                                                          // Decrement character count
        }

        if (ASSERT_EOL(character) || ASSERT_BUFFER_FULL(character_count))               // Validate
        {
            userData->inputString[character_count] = '\0';                              // Indicate string has been complete and return
            return;
        }

        if (ASSERT_PRINTABLE(character))                                                // Validate
        {
            userData->inputString[character_count] = character;                         // Append character to user input buffer
        }
    }
}

/**
*      @brief Function to parse the contents of the input string
*      @param userData Pointer to userData structure
**/
void parseInputString(shellData_t *userData)
{
    uint16_t character_count;
    RESET(userData->count);                                                             // Initialise count to 0
    uint8_t delimiter_flag = 1;                                                         // To indicate encounter of delimiters (non-alphanumeric chars)

    for (character_count = 0; character_count < MAX_STRING_LENGTH; character_count++)
    {
        if (ASSERT_ALPHABET(userData->inputString[character_count]))                    // Validate
        {
            if (delimiter_flag && userData->count < 10)                                 // Validate
            {
                delimiter_flag = 0;                                                     // Reset flag to 0
                userData->position[userData->count] = character_count;
                userData->type[userData->count] = 'a';                                  // Set field type to alphabet
                userData->count++;                                                      // Increment count
            }
        }

        else if (ASSERT_NUMBER(userData->inputString[character_count]))                 // Validate
        {
            delimiter_flag = 0;                                                         // Reset flag to 0
            userData->position[userData->count] = character_count;
            userData->type[userData->count] = 'n';                                      // Set field type to numeric
            userData->count++;                                                          // increment count
        }

        else
        {
            delimiter_flag = 1;                                                         // Revert flag to 1
            userData->inputString[character_count] = '\0';                              // Indicate end of string
        }
    }
}

/**
*      @brief Function to retrieve string
*      @param userData Pointer to user data structure
*      @param fieldNumber position to retrieve from
*      @return char* pointer to the string at position
**/
char *getFieldString(shellData_t *userData, uint8_t fieldNumber)
{
    if (userData->count > fieldNumber)     return &userData->inputString[userData->position[fieldNumber]];
    else                                    return '\0';
}

/**
*      @brief Function to retrieve integer
*      @param userData Pointer to user data structure
*      @param fieldNumber position to retrieve from
*      @return int32_t value of integer at position
**/
int32_t getFieldInteger(shellData_t *userData, uint8_t fieldNumber)
{
    return atoi(&userData->inputString[userData->position[fieldNumber]]);
}

/**
*      @brief Function to determine if the  input string is a command or not
*      @param userData Pointer to user data structure
*      @param command a valid command to compare with
*      @param arg_count number of arguments expected for the command
*      @return true if valid command
*      @return false if invalid command
**/
bool isCommand(shellData_t *userData, const char *command, uint8_t arg_count)
{
    bool cmp = strcmp(getFieldString(userData, 0), command);

    if (userData->count >= arg_count && cmp == 0)   return true;
    else                                             return false;
}
