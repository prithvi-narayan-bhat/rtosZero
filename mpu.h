#ifndef MPU_H
#define MPU_H

void enableMPU(void);
void setBackgroundRules(void);
void allowFlashAccess(void);
void setupSramAccess(void);
void setSramAccessWindow(uint32_t *baseAdd, uint32_t size_in_bytes);

#endif
