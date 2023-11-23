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

/**
*      @brief Function to print a compound string
*      @param arg argument of the string
*      @param s2 message to print with argument
*      @param argType type of argument
**/
void print(void *arg, const char *s, argType_t argType)
{
    char dest[MAX_STRING_LENGTH];
    char converted[20];

    if (argType == INT)
    {
        itoa(*((uint32_t *)arg), converted);
        strcpy(dest, s);
        strcat(dest, converted);
    }

    if (argType == BOOL)
    {
        bool val = *(bool *)arg;
        strcpy(dest, s);

        if (!val)       strcat(dest, " OFF");
        else if (val)   strcat(dest, " ON");
    }

    if (argType == CHAR)
    {
        strcpy(dest, s);
        strcat(dest, (const char *)arg);
    }

    if (argType == HEX)
    {
        htoa(*((uint32_t *)arg), converted);
        strcpy(dest, s);
        strcat(dest, converted);
    }

    putsUart0(dest);
    putsUart0("\r\n");
}

/**
*      @brief Function to copy string to destination
*      @param s1 destination
*      @param s2 source
**/
void strcpy(char *s1, const char *s2)
{
    while (*s2)
    {
        *s1++ = *s2++;
    }
    *s1 = '\0';
}

/**
*      @brief Function to convert a given string to lower case
*      @param string to be converted
**/
void toLower(char *s1) {
    while (*s1)
    {
        if (*s1 >= 'A' && *s1 <= 'Z') *s1 += ('a' - 'A');
        s1++;
    }
}

/**
 *      @brief Function to convert string to boolean value
 *      @param string to convert to bool
 *      @return true
 *      @return false
 **/
bool toBool(char *string)
{
    toLower(string);
    if (!strcmp(string, "on"))          return true;
    else if (!strcmp(string, "off"))    return false;
    else if (!strcmp(string, "prio"))   return true;
    else if (!strcmp(string, "rr"))     return false;
    else                                return false;
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
char *itoa(int32_t number, char *string)
{
    char *s = string;
    if (number < 0)  *s++ = '-';
    else        number = -number;                                                       // Append negative sign to start of number

    *itoa_helper(s, number) = '\0';                                                     // Append NULL character to indicate end of string
    return string;
}

/**
 *      @brief Function to convert a hex string to ASCII
 *      @param hexValue hexadecimal Value to convert
 *      @param string Hex value converted to string
 *      @return char* converted string
 **/
char *htoa(uint32_t hexValue, char *string)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        uint8_t nibble = (hexValue >> (28 - 4 * i)) & 0xF;                              // Extract each nibble (4 bits) from the hexValue

        if (nibble <= 9)    *string++ = '0' + nibble;                                   // Convert the nibble to its ASCII representation
        else                *string++ = 'A' + (nibble - 10);
    }

    *string = '\0';                                                                     // Null-terminate the ASCII string
    return string;
}

/**
*      @brief Function to convert an ASCII string to a uint32_t hex number
*      @param s String to convert
*      @return uint32_t Hex value
**/
uint32_t asciiToHex(const char *s)
{
    uint32_t hexValue = 0;                                                              // Initialize the hex value to 0

    while (*s)
    {
        unsigned char c = *s++;                                                         // Get the next character from the ASCII string

        // Convert the character to a hexadecimal digit and update the hex value
        if (ASSERT_NUMBER(c))       hexValue = (hexValue << 4) | (c - '0');             // Check if character is a digit
        else if (c >= 'A' && c <= 'F')  hexValue = (hexValue << 4) | (c - 'A' + 10);    // Check if character belongs to A-F
        else if (c >= 'a' && c <= 'f')  hexValue = (hexValue << 4) | (c - 'a' + 10);    // Check if character belongs to a-f
        else                            return 0;                                       // Return 0 to indicate an error
    }

    return hexValue;                                                                    // Return the computed hex value
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
*      @brief Function to concatenate two strings
*      @param s1 string 1 and destination for concatenation
*      @param s2 string 2
*      @return char* address of concatenated string
**/
char *strcat(char *s1, const char *s2)
{
    while (*s1)     s1++;               // Find the end of string 1

    *s1++ = ' ';                        // Append a space

    while (*s2)     *s1++ = *s2++;      // Append string 2 to string 1

    *s1 = '\0';                         // Append null

    return s1;                          // Return
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
            if (delimiter_flag && userData->count < 10)                                 // Validate
            {
                delimiter_flag = 0;                                                     // Reset flag to 0
                userData->position[userData->count] = character_count;
                userData->type[userData->count] = 'n';                                  // Set field type to numeric
                userData->count++;                                                      // increment count
            }
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
*      @brief Function to determine if the input string is a command or not (case insensitive)
*      @param userData Pointer to user data structure
*      @param cmp a valid command to compare with
*      @param arg_count number of arguments expected for the command
*      @return true if valid command
*      @return false if invalid command
**/
bool isCommand(shellData_t *userData, const char *cmp, uint8_t arg_count)
{
    char *cmd = getFieldString(userData, 0);
    toLower(cmd);
    bool val = strcmp(cmd, cmp);

    if (userData->count >= arg_count && val == 0)   return true;
    else                                            return false;
}
