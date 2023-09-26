/**
*      @file mpu.c
*      @author Prithvi Bhat
*      @brief Functions handling memory management in rtosZero
**/

/**
*      Cache-ability
*       Peripherals -> None => 00
*       Data        -> WBWA => 01
*       Control Reg -> WT   => 10
*
*       Memory Map
*
*   |    Start   |     End    | Description                         |
*   |------------|------------|-------------------------------------|
*   | 0x00000000 | 0x0003FFFF | On-chip Flash                       |
*   | 0x00040000 | 0x1FFFFFFF | Reserved                            |
*   | 0x20000000 | 0x20007FFF | Bit-banded on-chip SRAM             |
*   | 0x20008000 | 0x21FFFFFF | Reserved                            |
*   | 0x22000000 | 0x220FFFFF | Bit-band alias of bit-banded        |
*   |            |            | on-chip SRAM starting at 0x20000000 |
*   | 0x22100000 | 0x3FFFFFFF | Reserved                            |
*   | 0x40000000 | 0x40000FFF | Watchdog timer 0                    |
*   | 0x40001000 | 0x40001FFF | Watchdog timer 1                    |
*   | 0x40002000 | 0x40003FFF | Reserved                            |
*   | 0x40004000 | 0x40004FFF | GPIO Port A                         |
*   | 0x40005000 | 0x40005FFF | GPIO Port B                         |
**/

#include "tm4c123gh6pm.h"
#include <inttypes.h>

/**
*      @brief Function to enable MPU
**/
void enableMPU(void)
{
    NVIC_MPU_CTRL_R |= NVIC_MPU_CTRL_ENABLE;
    // NVIC_MPU_CTRL_R |= NVIC_MPU_CTRL_PRIVDEFEN;
}

/**
*      @brief Set the Background Rules for entire memory region
*               This will be overridden as subsequent regions are carved out and their own rules are implemented
**/
void setBackgroundRules(void)
{
    NVIC_MPU_NUMBER_R   |= NVIC_MPU_NUMBER_S;               // Set region number

    NVIC_MPU_BASE_R     |= 0x00000000;                      // Set base address as 0 (unnecessary since the size will be set to 4GB in attributes)

    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_XN;                // Enable universal execute never
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_AP_F;              // Full memory access
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_TEX_N;             // Normal Type Extension
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_SHAREABLE;         // Shareable
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_CACHEABLE;         // Cacheable
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_BUFFRABLE;         // Buffer-able
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_SIZE_FULL;         // Apply rules for full 4Gb space
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_ENABLE;            // Enable region
}

/**
*      @brief Function to allow access to Flash region
*           Non-sharable, non-buffer-able
**/
void allowFlashAccess(void)
{
    NVIC_MPU_NUMBER_R   |= NVIC_MPU_NUMBER_FLASH;           // Set Flash region number

    NVIC_MPU_BASE_R     |= 0x00000000;                      // Flash address

    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_AP_F;              // Full memory access
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_CACHEABLE;         // Cacheable
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_SIZE_FLASH;        // Apply rules to Flash 0x00000 to 0x3FFFF
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_ENABLE;            // Enable region
}

/**
 *      @brief Function to allow access to Peripheral region
 *              Shareable, Buffer-able and non-cache-able
 **/
void allowPeripheralAccess(void)
{
    NVIC_MPU_NUMBER_R   |= NVIC_MPU_NUMBER_PERIPHERAL;      // Set Peripheral region number

    NVIC_MPU_BASE_R     |= 0x40000000;                      // Peripheral address

    NVIC_MPU_ATTR_R     &= ~NVIC_MPU_ATTR_XN;               // Disable universal execute never
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_AP_F;              // Full memory access
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_TEX_N;             // Normal Type Extension
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_BUFFRABLE;         // Buffer-able
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_SHAREABLE;         // Shareable
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_SIZE_PERIPHERAL;   // Apply rules to Peripherals and Peripheral bit-band region
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_ENABLE;            // Enable region
}
