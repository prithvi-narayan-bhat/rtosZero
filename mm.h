// Memory manager functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef MM_H_
#define MM_H_

#define NUM_SRAM_REGIONS 5

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void * mallocFromHeap(uint32_t size_in_bytes);
void generateSrdMasks(uint32_t *baseAdd, uint32_t size_in_bytes, uint8_t *subRegionMap);
void applySrdRules(uint8_t *subRegionMap);
void initMpu(void);

#endif
