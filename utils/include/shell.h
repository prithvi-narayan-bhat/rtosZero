#ifndef SHELL_H
#define SHELL_H

typedef struct
{
    uint8_t task;
    uint32_t pid;
    uint32_t cpuTime;
    char name[10];
} psInfo_t;

typedef struct
{
    bool lock;
    uint16_t queueSize;
    uint32_t processQueue[2];
    char processName[2][10];
    uint32_t lockedBy;
    char lockedByName[10];
} mutexInfo_t;

typedef struct
{
    uint16_t count;
    uint16_t queueSize;
    uint32_t processQueue[2];
    char processName[2][10];
} semaphoreInfo_t;

void shell(void);

#endif
