#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

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


void *malloc_from_heap(int size_in_bytes);
void free(void *baseAdd);

#endif
