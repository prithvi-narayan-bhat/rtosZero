/**
*      @file mpu.c
*      @author Prithvi Bhat
*      @brief Functions handling memory management in rtosZero
**/

#include "memory.h"

/**
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
 *      @brief Function to set access rules for the SRAM region
 *              Shareable, Cacheable
 *      Region Map
 *      | Region | Start Addr | End Addr   | AP  |
 *      |--------|------------|------------|-----|
 *      | Base   | 0x00000000 | 0xFFFFFFFF | 011 |
 *      | Flash  | 0x00000000 | 0x0003FFFF | 011 |
 *      | OS+GV  | 0x20000000 | 0x20000FFF | 001 |
 *      | 4K-1   | 0x20001000 | 0x20001FFF | 011 |
 *      | 8K-1   | 0x20002000 | 0x20003FFF | 011 |
 *      | 4K-2   | 0x20004000 | 0x20004FFF | 011 |
 *      | 4K-3   | 0x20005000 | 0x20005FFF | 011 |
 *      | 8K-2   | 0x20006000 | 0x20007FFF | 011 |
 **/
void setupSramAccess(void)
{
/**
*      @brief Macro to set repetitive region attributes
**/
#ifndef UPDATE_SRAM_MPU_RULES
#define UPDATE_SRAM_MPU_RULES(regionNumber, address, privilege, srd, size)                          \
    ({                                                                                              \
        NVIC_MPU_NUMBER_R   = NVIC_MPU_NUMBER_SRAM_##regionNumber;  /* Set SRAM region number */    \
        NVIC_MPU_BASE_R     = address;                              /* SRAM address */              \
        NVIC_MPU_ATTR_R     = NVIC_MPU_ATTR_XN;                     /* Enable execute never */      \
        NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_##privilege;           /* Privileged access */         \
        NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_TEX_N;                 /* Normal Type Extension */     \
        NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_SHAREABLE;             /* Shareable */                 \
        NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_CACHEABLE;             /* Cacheable */                 \
        NVIC_MPU_ATTR_R     |= srd;                                 /* Disable sub-regions */       \
        NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_SIZE_SRAM_##size;      /* Apply rules  */              \
        NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_ENABLE;                /* Enable region */             \
    })

    UPDATE_SRAM_MPU_RULES(OS, REGION_4K0_BASE_ADDR, AP_K, 0, 4K);                       // SRAM base   0 => 4K starting from 0x20000000
    UPDATE_SRAM_MPU_RULES(4K1, REGION_4K1_BASE_ADDR, AP_F, NVIC_MPU_ATTR_SRD_M, 4K);    // SRAM Region 1 => 4K starting from 0x20001000
    UPDATE_SRAM_MPU_RULES(8K1, REGION_8K1_BASE_ADDR, AP_F, NVIC_MPU_ATTR_SRD_M, 8K);    // SRAM Region 2 => 8K starting from 0x20002000
    UPDATE_SRAM_MPU_RULES(4K2, REGION_4K2_BASE_ADDR, AP_F, NVIC_MPU_ATTR_SRD_M, 4K);    // SRAM Region 3 => 4K starting from 0x20004000
    UPDATE_SRAM_MPU_RULES(4K3, REGION_4K3_BASE_ADDR, AP_F, NVIC_MPU_ATTR_SRD_M, 4K);    // SRAM Region 4 => 4K starting from 0x20005000
    UPDATE_SRAM_MPU_RULES(8K2, REGION_8K2_BASE_ADDR, AP_F, NVIC_MPU_ATTR_SRD_M, 8K);    // SRAM Region 5 => 8K starting from 0x20006000

#undef UPDATE_SRAM_MPU_RULES
#endif
}

/**
*      @brief Function to get the Mask to enable subregion bits
*      @param subRegionCount Number of sub regions
*      @param subRegionStart starting location of the first subregion to enable
*      @return uint32_t mask value
**/
uint32_t getMask(uint8_t subRegionCount, uint8_t subRegionStart)
{
    uint32_t mask;
    mask = ((1 << subRegionCount) - 1);                     // Create a mask of the number of regions to enable
    mask <<= subRegionStart;                                // Move the bits to the appropriate subregion position
    mask = ~mask;                                           // Invert the value of the bits (0 is enable and 1 is disable)
    mask <<= 8;                                             // Move the bits to position of the SRD bits in the register
    mask |= 0xFF;                                           // Add 1 to the left-shifted bits
    return (mask & 0xFFFFFFFF);                             // Return value
}

/**
*      @brief Function to enable SRAM regions
*      @param baseAdd address of the requested space
*      @param regionAdd base address of the parent region of the requested space
*      @param size_in_bytes requested space in bytes
*      @param minBlockSize minimum bloc allocation in the requested region
*      @param sramRegion location of requested space within one of five regions
**/
void enableSubRegions(uint32_t baseAdd, uint32_t regionAdd, uint32_t size_in_bytes, uint16_t minBlockSize, uint8_t sramRegion)
{
    uint8_t subRegionStart = ((uint32_t)baseAdd - regionAdd) / minBlockSize;    // Determine start of the subregions to enable
    uint8_t subRegionCount = size_in_bytes / minBlockSize;                      // Get the number of subregions to enable

    NVIC_MPU_NUMBER_R   = sramRegion;                                           // Set SRAM region number
    NVIC_MPU_ATTR_R     |= NVIC_MPU_ATTR_AP_F;                                  // Privileged access
    NVIC_MPU_ATTR_R     &= getMask(subRegionCount, subRegionStart);             // Disable sub-regions
}

/**
*      @brief Set the Sram Access Window object
*      @param baseAdd Address to allocate from
*      @param size_in_bytes Number of byte to allocate
**/
void setSramAccessWindow(uint32_t *baseAdd, uint32_t size_in_bytes)
{
    // First 1.5K region
    if (baseAdd == 0x20001E00)
    {
        enableSubRegions((uint32_t)0x20001E00, REGION_4K1_BASE_ADDR, 512, BLOCK_SIZE_1, NVIC_MPU_NUMBER_SRAM_4K1);
        enableSubRegions((uint32_t)0x20002000, REGION_8K1_BASE_ADDR, 1024, BLOCK_SIZE_2, NVIC_MPU_NUMBER_SRAM_8K1);
        return;
    }

    else if (baseAdd == 0x20003C00)
    {
        enableSubRegions((uint32_t)0x20003C00, REGION_8K1_BASE_ADDR, 1024, BLOCK_SIZE_2, NVIC_MPU_NUMBER_SRAM_8K1);
        enableSubRegions((uint32_t)0x20004000, REGION_4K2_BASE_ADDR, 512, BLOCK_SIZE_1, NVIC_MPU_NUMBER_SRAM_4K2);
        return;
    }

    else if (baseAdd == 0x20005E00)
    {
        enableSubRegions((uint32_t)0x20005E00, REGION_4K3_BASE_ADDR, 512, BLOCK_SIZE_1, NVIC_MPU_NUMBER_SRAM_4K3);
        enableSubRegions((uint32_t)0x20006000, REGION_8K2_BASE_ADDR, 1024, BLOCK_SIZE_2, NVIC_MPU_NUMBER_SRAM_8K2);
        return;
    }

    // Determine the region within SRAM
    else if ((uint32_t)baseAdd < REGION_4K1_TOP_ADDR)                           // Region 1 => 4K
    {
        enableSubRegions((uint32_t)baseAdd, REGION_4K1_BASE_ADDR, size_in_bytes, BLOCK_SIZE_1, NVIC_MPU_NUMBER_SRAM_4K1);
        return;
    }

    else if ((uint32_t)baseAdd < REGION_8K1_TOP_ADDR)                           // Region 2 => 8K
    {
        enableSubRegions((uint32_t)baseAdd, REGION_8K1_BASE_ADDR, size_in_bytes, BLOCK_SIZE_2, NVIC_MPU_NUMBER_SRAM_8K1);
        return;
    }

    else if ((uint32_t)baseAdd < REGION_4K2_TOP_ADDR)                           // Region 3 => 4K
    {
        enableSubRegions((uint32_t)baseAdd, REGION_4K2_BASE_ADDR, size_in_bytes, BLOCK_SIZE_1, NVIC_MPU_NUMBER_SRAM_4K2);
        return;
    }

    else if ((uint32_t)baseAdd < REGION_4K3_TOP_ADDR)                           // Region 4 => 4K
    {
        enableSubRegions((uint32_t)baseAdd, REGION_4K3_BASE_ADDR, size_in_bytes, BLOCK_SIZE_1, NVIC_MPU_NUMBER_SRAM_4K3);
        return;
    }

    else if ((uint32_t)baseAdd < REGION_8K2_TOP_ADDR)                           // Region 5 => 8K
    {
        enableSubRegions((uint32_t)baseAdd, REGION_8K2_BASE_ADDR, size_in_bytes, BLOCK_SIZE_2, NVIC_MPU_NUMBER_SRAM_8K2);
    }
}
