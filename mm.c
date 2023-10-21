// Memory manager functions
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
#include "tm4c123gh6pm.h"
#include "mm.h"

// User added
#define NULL        0x00000000

#define BLOCK_SIZE_1    512
#define BLOCK_SIZE_2    1024
#define BLOCK_SIZE_3    1536

#define REGION_4K0_BASE_ADDR    0x20000000
#define REGION_4K1_BASE_ADDR    0x20001000
#define REGION_4K2_BASE_ADDR    0x20004000
#define REGION_4K3_BASE_ADDR    0x20005000
#define REGION_8K1_BASE_ADDR    0x20002000
#define REGION_8K2_BASE_ADDR    0x20006000


#define REGION_4K0_TOP_ADDR     0x20000FFF
#define REGION_4K1_TOP_ADDR     0x20001FFF
#define REGION_8K1_TOP_ADDR     0x20003FFF
#define REGION_4K2_TOP_ADDR     0x20004FFF
#define REGION_4K3_TOP_ADDR     0x20005FFF
#define REGION_8K2_TOP_ADDR     0x20007FFF

#define NVIC_MPU_NUMBER_SRAM_8K2    0x00000007  // SRAM region 5        (User added)
#define NVIC_MPU_NUMBER_SRAM_4K3    0x00000006  // SRAM region 4        (User added)
#define NVIC_MPU_NUMBER_SRAM_4K2    0x00000005  // SRAM region 3        (User added)
#define NVIC_MPU_NUMBER_SRAM_8K1    0x00000004  // SRAM region 2        (User added)
#define NVIC_MPU_NUMBER_SRAM_4K1    0x00000003  // SRAM region 1        (User added)
#define NVIC_MPU_NUMBER_SRAM_OS     0x00000002  // SRAM region 0        (User added)
#define NVIC_MPU_NUMBER_FLASH       0x00000001  // Flash region         (User added)
#define NVIC_MPU_ATTR_AP_K          0x01000000  // Kernel Full Access       (User added)
#define NVIC_MPU_ATTR_AP_F          0x03000000  // Access Privilege Full    (User added)
#define NVIC_MPU_ATTR_TEX_N         0x00000000  // Type Extension Mask      (User added)
#define NVIC_MPU_ATTR_SIZE_FULL     (31 << 1)   // Region Size Mask Full    (User added)
#define NVIC_MPU_ATTR_SIZE_FLASH    (17 << 1)   // Region Size Mask Flash   (User added)
#define NVIC_MPU_ATTR_SIZE_SRAM_8K  (12 << 1)   // Region Size Mask SRAM    (User added)
#define NVIC_MPU_ATTR_SIZE_SRAM_4K  (11 << 1)   // Region Size Mask SRAM    (User added)

#define TOTAL_REGIONS   40

#define BLOCK_4K1_START 0
#define BLOCK_4K1_END   7
#define BLOCK_8K1_START 8
#define BLOCK_8K1_END   15
#define BLOCK_4K2_START 16
#define BLOCK_4K2_END   23
#define BLOCK_4K3_START 24
#define BLOCK_4K3_END   31
#define BLOCK_8K2_START 32
#define BLOCK_8K2_END   39

#define SUBREGIONS_512(size_in_bytes)    (size_in_bytes / BLOCK_SIZE_1)   // Determine the number of 512B sub regions required for the allotment
#define SUBREGIONS_1024(size_in_bytes)   (size_in_bytes / BLOCK_SIZE_2)   // Determine the number of 1024B sub regions required for the allotment

#define RETURN_VALID    if (retVal != NULL) return (void *)retVal
#define RETURN_INVALID  return (void *)NULL

/**
*      @brief Structure to hold the metadata of the Heap
**/
typedef struct
{
    void *address;                                              // Base of the allocation
    uint8_t subRegions;                                         // Number of subregions allocated here
} heapMetadata_t;

// Global variables
uint8_t heapTop_g = 0, allotment_g[TOTAL_REGIONS] = {0, };      // A ledger to keep track allocated subregions, initialised to 0
heapMetadata_t heapMetadata_g[TOTAL_REGIONS] = {{0, 0}, };      // Initialise allotment metadata

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

/**
 *      @brief Get the Allocation object
 *      @param allotment_g  pointer to the heap ledger array
 *      @param heapMetadata_g pointer to the metadata structure array
 *      @param size of the requested space in bytes
 *      @param subRegions number of subregions occupied by the requested space
 *      @param startRange start index of the region (block offset from 0)
 *      @param endRange end index of the region (block offset from 0)
 *      @param baseAddr base address of the region
 *      @param offset from 0
 *      @param blockSize size of each subregion in the region
 *      @return void* an address to the base of allocated heap space
 **/
void *getAllocation(uint8_t subRegions, uint8_t startRange, uint8_t endRange, uint32_t baseAddr, int16_t offset, uint16_t blockSize)
{
    uint8_t fragmented = 0, emptySubRegions = 0, index;

    // Check for space in the first 8K region
    for (index = startRange; index <= endRange; index++)                            // +-1 to leave space for 1.5K blocks
    {
        if (!(allotment_g[index]))                                                  // Check if space is unallocated
        {
            if (fragmented)                                                         // Clear fragmentation flags if set
            {
                fragmented = 0;
                emptySubRegions = 0;                                                // Reset count of contiguous empty spaces
            }

            emptySubRegions++;                                                      // Keep count to ensure space will fit

            if (emptySubRegions >= subRegions)                                      // Check if contiguous empty spaces match the required number of regions
            {
                while (emptySubRegions > 0)
                {
                    if (index > 1)  allotment_g[index--] = 1;                       // To indicate that the space has been alloted
                    else        allotment_g[index] = 1;                             // To indicate that the space has been alloted
                    emptySubRegions--;
                }

                heapMetadata_g[heapTop_g].subRegions = subRegions;                  // Record size and allocated address
                heapMetadata_g[heapTop_g].address = (void *)(baseAddr + ((index + (offset)) * blockSize));

                return (void *)heapMetadata_g[heapTop_g++].address;                 // Return the address
            }
        }
        else
        {
            fragmented = 1;                                                         // Set fragmented flag
        }
    }
    return (void *)NULL;
}

// REQUIRED: add your malloc code here and update the SRD bits for the current thread
void * mallocFromHeap(uint32_t size_in_bytes)
{
    void *retVal;

    // Standardize size
    if      (size_in_bytes <= BLOCK_SIZE_1)  size_in_bytes = BLOCK_SIZE_1;
    else if (size_in_bytes <= BLOCK_SIZE_2)  size_in_bytes = BLOCK_SIZE_2;
    else    (size_in_bytes = (size_in_bytes <= BLOCK_SIZE_3) ? BLOCK_SIZE_3 : ((size_in_bytes % BLOCK_SIZE_2) ? ((size_in_bytes / BLOCK_SIZE_2) + 1) * BLOCK_SIZE_2 : size_in_bytes));

    // If request is for one of three 1.5K intersections
    if (size_in_bytes == BLOCK_SIZE_3)
    {
        retVal = getAllocation((2), (BLOCK_4K1_END), (BLOCK_8K1_START), REGION_4K1_BASE_ADDR, 1, BLOCK_SIZE_1);
        RETURN_VALID;

        retVal = getAllocation((2), (BLOCK_8K1_END), (BLOCK_4K2_START), REGION_8K1_BASE_ADDR, -7, BLOCK_SIZE_2);
        RETURN_VALID;

        retVal = getAllocation((2), (BLOCK_4K3_END), (BLOCK_8K2_START), REGION_4K3_BASE_ADDR, -23, BLOCK_SIZE_1);
        RETURN_VALID;
    }

    // Make an allotment from one of the three 4K blocks
    if (SUBREGIONS_512(size_in_bytes) < SUBREGIONS_1024(size_in_bytes) || !SUBREGIONS_1024(size_in_bytes))
    {
        retVal = getAllocation((SUBREGIONS_512(size_in_bytes)), (BLOCK_4K1_START), (BLOCK_4K1_END - 1), REGION_4K1_BASE_ADDR, 0, BLOCK_SIZE_1);
        RETURN_VALID;

        retVal = getAllocation((SUBREGIONS_512(size_in_bytes)), (BLOCK_4K2_START + 1), BLOCK_4K2_END, REGION_4K2_BASE_ADDR, -15, BLOCK_SIZE_1);
        RETURN_VALID;

        retVal = getAllocation((SUBREGIONS_512(size_in_bytes)), (BLOCK_4K3_START), (BLOCK_4K3_END - 1), REGION_4K3_BASE_ADDR, -22, BLOCK_SIZE_1);
        RETURN_VALID;
    }

    // Make an allotment from one of the two 8K blocks
    else
    {
        retVal = getAllocation((SUBREGIONS_1024(size_in_bytes)), (BLOCK_8K1_START + 1), (BLOCK_8K1_END - 1), REGION_8K1_BASE_ADDR, -7, BLOCK_SIZE_2);
        RETURN_VALID;

        retVal = getAllocation((SUBREGIONS_1024(size_in_bytes)), (BLOCK_8K2_START + 1), (BLOCK_8K2_END), REGION_8K2_BASE_ADDR, -31, BLOCK_SIZE_2);
        RETURN_VALID;
    }

    RETURN_INVALID;
}

// REQUIRED: add your custom MPU functions here (eg to return the srd bits)
/**
*      @brief Function to enable MPU
**/
void enableMPU(void)
{
    NVIC_MPU_CTRL_R |= NVIC_MPU_CTRL_ENABLE;
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
    if ((uint32_t)baseAdd == 0x20001E00)
    {
        enableSubRegions((uint32_t)0x20001E00, REGION_4K1_BASE_ADDR, 512, BLOCK_SIZE_1, NVIC_MPU_NUMBER_SRAM_4K1);
        enableSubRegions((uint32_t)0x20002000, REGION_8K1_BASE_ADDR, 1024, BLOCK_SIZE_2, NVIC_MPU_NUMBER_SRAM_8K1);
        return;
    }

    else if ((uint32_t)baseAdd == 0x20003C00)
    {
        enableSubRegions((uint32_t)0x20003C00, REGION_8K1_BASE_ADDR, 1024, BLOCK_SIZE_2, NVIC_MPU_NUMBER_SRAM_8K1);
        enableSubRegions((uint32_t)0x20004000, REGION_4K2_BASE_ADDR, 512, BLOCK_SIZE_1, NVIC_MPU_NUMBER_SRAM_4K2);
        return;
    }

    else if ((uint32_t)baseAdd == 0x20005E00)
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

// REQUIRED: initialize MPU here
void initMpu(void)
{
    // REQUIRED: call your MPU functions here
    enableMPU();
    setBackgroundRules();
    allowFlashAccess();
    setupSramAccess();
}
