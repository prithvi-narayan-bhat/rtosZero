#include "memory.h"

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

#define SUBREGIONS_512(size)    (size / BLOCK_SIZE_1)   // Determine the number of 512B sub regions required for the allotment
#define SUBREGIONS_1024(size)   (size / BLOCK_SIZE_2)   // Determine the number of 1024B sub regions required for the allotment

/**
*      @brief Structure to hold the metadata of the Heap
**/
typedef struct
{
    uint32_t *address;          // Base of the allocation
    uint16_t size;              // Size of the allocation starting at the base
} heapMetadata_t;

uint8_t heapTop_g = 0;          // A ledger to keep track allocated subregions

/**
 *      @brief Get the Allocation object
 *      @param allotment_s  pointer to the heap ledger array
 *      @param heapMetadata_s pointer to the metadata structure array
 *      @param size of the requested space in bytes
 *      @param subRegions number of subregions occupied by the requested space
 *      @param startRange start index of the region (block offset from 0)
 *      @param endRange end index of the region (block offset from 0)
 *      @param baseAddr base address of the region
 *      @param offset from 0
 *      @param blockSize size of each subregion in the region
 *      @return void* an address to the base of allocated heap space
 **/
void *getAllocation(uint8_t *allotment_s, heapMetadata_t *heapMetadata_s, uint32_t size, uint8_t subRegions, uint8_t startRange, uint8_t endRange, uint32_t baseAddr, int16_t offset, uint16_t blockSize)
{
    uint8_t fragmented = 0, emptySubRegions = 0, index;

    // Check for space in the first 8K region
    for (index = startRange; index <= endRange; index++)      // +-1 to leave space for 1.5K blocks
    {
        if (!(allotment_s[index]))                                                  // Check if space is unallocated
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
                    if (index > 1)  allotment_s[index--] = 1;                       // To indicate that the space has been alloted
                    else        allotment_s[index] = 1;                             // To indicate that the space has been alloted
                    emptySubRegions--;
                }

                heapMetadata_s[heapTop_g].size = size;                              // Record size and allocated address
                heapMetadata_s[heapTop_g].address = (uint32_t *)(baseAddr + ((index + (offset)) * blockSize));

                return (void *)heapMetadata_s[heapTop_g++].address;                 // Return the address
            }
        }
        else
        {
            fragmented = 1;                                                         // Set fragmented flag
        }
    }
    return (void *)NULL;
}

/**
 *      @brief Function to allocate requested memory from heap
 *      @param size requested in bytes
 *      @return void* address to base of the allocated space
 **/
void *malloc_from_heap(int32_t size)
{
    static uint8_t allotment_s[TOTAL_REGIONS] = {0, };                  // Initialise allotment record to zero
    static heapMetadata_t heapMetadata_s[TOTAL_REGIONS] = {{0, 0}, };   // Initialise allotment metadata

    uint8_t subRegions, block;
    int32_t *retVal;

    // Standardize size
    if      (size <= BLOCK_SIZE_1)  size = BLOCK_SIZE_1;
    else if (size <= BLOCK_SIZE_2)  size = BLOCK_SIZE_2;
    else    (size = (size <= BLOCK_SIZE_3) ? BLOCK_SIZE_3 : ((size % BLOCK_SIZE_2) ? ((size / BLOCK_SIZE_2) + 1) * BLOCK_SIZE_2 : size));

    // If request is for one of three 1.5K intersections
    if (size == BLOCK_SIZE_3)
    {
        retVal = getAllocation(allotment_s, heapMetadata_s, size, (2), (BLOCK_4K1_END), (BLOCK_8K1_START), REGION_4K1_BASE_ADDR, 1, BLOCK_SIZE_1);
        if (retVal != NULL) return (void *)retVal;

        retVal = getAllocation(allotment_s, heapMetadata_s, size, (2), (BLOCK_8K1_END), (BLOCK_4K2_START), REGION_8K1_BASE_ADDR, -7, BLOCK_SIZE_2);
        if (retVal != NULL) return (void *)retVal;

        retVal = getAllocation(allotment_s, heapMetadata_s, size, (2), (BLOCK_4K3_END), (BLOCK_8K2_START), REGION_4K3_BASE_ADDR, -23, BLOCK_SIZE_1);
        if (retVal != NULL) return (void *)retVal;
    }

    // Make an allotment from one of the three 4K blocks
    else if (SUBREGIONS_512(size) < SUBREGIONS_1024(size) || !SUBREGIONS_1024(size))
    {
        retVal = getAllocation(allotment_s, heapMetadata_s, size, (SUBREGIONS_512(size)), (BLOCK_4K1_START), (BLOCK_4K1_END - 1), REGION_4K1_BASE_ADDR, 0, BLOCK_SIZE_1);
        if (retVal != NULL) return (void *)retVal;

        retVal = getAllocation(allotment_s, heapMetadata_s, size, (SUBREGIONS_512(size)), (BLOCK_4K2_START + 1), BLOCK_4K2_END, REGION_4K2_BASE_ADDR, -15, BLOCK_SIZE_1);
        if (retVal != NULL) return (void *)retVal;

        retVal = getAllocation(allotment_s, heapMetadata_s, size, (SUBREGIONS_512(size)), (BLOCK_4K3_START), (BLOCK_4K3_END - 1), REGION_4K3_BASE_ADDR, -22, BLOCK_SIZE_1);
        if (retVal != NULL) return (void *)retVal;
    }

    // Make an allotment from one of the two 8K blocks
    else
    {
        retVal = getAllocation(allotment_s, heapMetadata_s, size, (SUBREGIONS_1024(size)), (BLOCK_8K1_START + 1), (BLOCK_8K1_END - 1), REGION_8K1_BASE_ADDR, -7, BLOCK_SIZE_2);
        if (retVal != NULL) return (void *)retVal;

        retVal = getAllocation(allotment_s, heapMetadata_s, size, (SUBREGIONS_1024(size)), (BLOCK_8K2_START + 1), (BLOCK_8K2_END), REGION_8K2_BASE_ADDR, -31, BLOCK_SIZE_2);
        if (retVal != NULL) return (void *)retVal;
    }

    return (void *)NULL;
}
