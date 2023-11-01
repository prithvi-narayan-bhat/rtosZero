// Kernel functions
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
#include "kernel.h"
#include "faults.h"
#include "strings.h"
#include "systemRegisters.h"

//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------

// mutex
mutex mutexes[MAX_MUTEXES];

// semaphore
semaphore semaphores[MAX_SEMAPHORES];

// task states
#define STATE_INVALID           0 // no task
#define STATE_STOPPED           1 // stopped, can be resumed
#define STATE_UNRUN             2 // task has never been run
#define STATE_READY             3 // has run, can resume at any time
#define STATE_DELAYED           4 // has run, but now awaiting timer
#define STATE_BLOCKED_MUTEX     5 // has run, but now blocked by semaphore
#define STATE_BLOCKED_SEMAPHORE 6 // has run, but now blocked by semaphore

// task
uint8_t taskCurrent = 0;          // index of last dispatched task
uint8_t taskCount = 0;            // total number of valid tasks

// control
bool priorityScheduler = true;    // priority (true) or round-robin (false)
bool priorityInheritance = false; // priority inheritance for mutexes
bool preemption = false;          // preemption (true) or cooperative (false)

// tcb
#define NUM_PRIORITIES   8
struct _tcb
{
    uint8_t state;                 // see STATE_ values above
    void *pid;                     // used to uniquely identify thread (add of task fn)
    void *spInit;                  // original top of stack
    void *sp;                      // current stack pointer
    uint8_t priority;              // 0=highest
    uint8_t currentPriority;       // 0=highest (needed for pi)
    uint32_t ticks;                // ticks until sleep complete
    uint8_t srd[NUM_SRAM_REGIONS]; // MPU subregion disable bits
    char name[16];                 // name of task used in ps command
    uint8_t mutex;                 // index of the mutex in use or blocking the thread
    uint8_t semaphore;             // index of the semaphore that is blocking the thread
} tcb[MAX_TASKS];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool initMutex(uint8_t mutex)
{
    bool ok = (mutex < MAX_MUTEXES);
    if (ok)
    {
        mutexes[mutex].lock = false;
        mutexes[mutex].lockedBy = 0;
    }
    return ok;
}

bool initSemaphore(uint8_t semaphore, uint8_t count)
{
    bool ok = (semaphore < MAX_SEMAPHORES);
    {
        semaphores[semaphore].count = count;
    }
    return ok;
}

/**
 *      @brief Initialisation for sysTicks
 **/
void initSysTick(void)
{
    NVIC_ST_RELOAD_R = 39999;                   // sys_clock * 1 * 10^-3 for a 1ms tick
    NVIC_ST_CURRENT_R = NVIC_ST_CURRENT_M;      // Clear current value by writing any value

    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_CLK_SRC;     // Enable system clock source for Systick operation
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_INTEN;       // Enable systick interrupts
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE;      // Start systick
}

/**
 *      @brief Function to initialise the Task Control Block before starting any threads
 **/
void initRtos(void)
{
    uint8_t i;

    initSysTick();                              // Initialise system ticks

    taskCount = 0;                              // No tasks running

    for (i = 0; i < MAX_TASKS; i++)             // Clear out tcb records
    {
        tcb[i].state = STATE_INVALID;
        tcb[i].pid = 0;
    }
}

// REQUIRED: Implement prioritization to NUM_PRIORITIES
uint8_t rtosScheduler(void)
{
    bool ok;
    static uint8_t task = 0xFF;
    ok = false;
    while (!ok)
    {
        task++;
        if (task >= MAX_TASKS)  task = 0;

        ok = (tcb[task].state == STATE_READY || tcb[task].state == STATE_UNRUN);
    }
    return task;
}

/**
 *      @brief Function to spawn a method in unprivileged mode
 *      @param fn address of the function to be spawned
 *              The argument being passed will ensure that the PC has the value of the method to be executed
 **/
void spawn(_fn fn)
{
    disablePrivilegedMode();                // Enable the TMPL bit in the CONTROL register
    fn();                                   // Call the method
}

/**
 *      @brief Function to start the OS
 *              It get the first method to be executed from the Scheduler
 *              Sets the necessary bits in the MPU_SRD regions, loads the PSP and sets the ASP bit
 *              It calls a function accepting the method address to be spawned
 **/
void startRtos(void)
{
    static _fn fn;                          // Must be declared statically to ensure it is available even on setting ASP bit
    uint8_t task = rtosScheduler();         // Invoke RTOS scheduler

    void *taskPID = tcb[task].pid;          // Create a function to load the TMPL bit and start the task
    tcb[task].state = STATE_READY;          // Update the status of the thread
    fn = (_fn)taskPID;                      // Assign locally

    applySrdRules(tcb[task].srd);           // Apply the SRD rules specific to the first thread
    stageMethod((uint32_t)tcb[task].sp);    // Load stack pointer onto PSP register and set ASP bit in Control register

    spawn(fn);                              // Invoke function to spawn method
}

/**
 *      @brief Create a Thread object
 *      @param fn pointer to the thread to be created
 *      @param name of the thread to create
 *      @param priority to be allocated to the thread
 *      @param stackBytes number of bytes to be allocated to the thread
 *      @return true status if creation successful
 *      @return false status if creation unsuccessful
 **/
bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes)
{
    bool ok = false, found = false;
    uint8_t i = 0;

    if (taskCount < MAX_TASKS)
    {
        // Ensure "fn" not already in list (prevent re-entrancy)
        while (!found && (i < MAX_TASKS))
        {
            found = (tcb[i++].pid ==  fn);
        }

        if (!found)
        {
            // Find first available TCB record
            i = 0;
            while (tcb[i].state != STATE_INVALID) {i++;}

            // An empty record has been found
            void *ptr = mallocFromHeap(stackBytes);                         // Request memory from heap

            strcpy(tcb[i].name, name);                                      // Store name
            tcb[i].state        = STATE_UNRUN;                              // Store initial state as Un-Run
            tcb[i].pid          = fn;                                       // Store PID
            tcb[i].sp           = (void *)((uint32_t)ptr + stackBytes);     // ptr + (size in hex)
            tcb[i].spInit       = (void *)((uint32_t)ptr + stackBytes);     // ptr + (size in hex)
            tcb[i].priority     = priority;                                 // Store the requested PID

            generateSrdMasks(ptr, stackBytes, tcb[i].srd);                  // Store SRD masks in the TCB

            taskCount++;                                                    // Increment record of task count
            ok = true;
        }
    }
    return ok;
}

// REQUIRED: modify this function to restart a thread
void restartThread(_fn fn)
{
}

// REQUIRED: modify this function to stop a thread
// REQUIRED: remove any pending semaphore waiting, unlock any mutexes
void stopThread(_fn fn)
{
}

// REQUIRED: modify this function to set a thread priority
void setThreadPriority(_fn fn, uint8_t priority)
{
}

// REQUIRED: modify this function to yield execution back to scheduler using pendsv
void yield(void)
{
    __asm(" SVC #0x00");                    // Trigger a Service call
}

// REQUIRED: modify this function to support 1ms system timer
// execution yielded back to scheduler until time elapses using pendsv
void sleep(uint32_t tick)
{
}

// REQUIRED: modify this function to lock a mutex using pendsv
void lock(int8_t mutex)
{
}

// REQUIRED: modify this function to unlock a mutex using pendsv
void unlock(int8_t mutex)
{
}

// REQUIRED: modify this function to wait a semaphore using pendsv
void wait(int8_t semaphore)
{
}

// REQUIRED: modify this function to signal a semaphore is available using pendsv
void post(int8_t semaphore)
{
}

// REQUIRED: modify this function to add support for the system timer
// REQUIRED: in preemptive code, add code to request task switch
void systickIsr(void)
{
}

// REQUIRED: in coop and preemptive, modify this function to add support for task switching
// REQUIRED: process UNRUN and READY tasks differently
/**
 *      @brief Function to handle context switching
 *              This is essentially an ISR and will be called automatically and performs the following:
 *              1. PUSH the status of the current context to the stack
 *              2. Request the scheduler for the next context to be executed
 *              3. Determine state of next context
 *              4. If state == STATE_UNRUN, follow same steps as in loading a new context
 *                  else POP the saved status of context from stack and load it
 **/
__attribute__((naked)) void pendSvIsr(void)
{
    __asm(" MRS     R0, PSP");                      // Load the PSP into a local register in the stack frame
    __asm(" STMDB   R0, {R4-R11, LR}");             // Store registers R4-R11 and LR in the stack frame

    tcb[taskCurrent].state = STATE_READY;           // Update old state to be ready

    tcb[taskCurrent].sp = (void *)getPSP();         // Store the PSP to the sp of the current task
    taskCurrent = rtosScheduler();                  // Invoke RTOS scheduler, get next task

    applySrdRules(tcb[taskCurrent].srd);            // Apply the SRD rules specific to the first thread
    loadPSP((uint32_t)tcb[taskCurrent].sp);         // Load the new PSP and execute

    switch (tcb[taskCurrent].state)
    {
        case STATE_UNRUN:
        {
            // Create a stack frame to trick the processor into thinking this thread was previously run
            uint32_t *psp = (uint32_t *)tcb[taskCurrent].sp; // Get the stack pointer
            *(psp - 1) = 0x01000000;                // Load the Thumb bit in the xPSR or things go south
            *(psp - 2) = (uint32_t)tcb[taskCurrent].pid;    // Store PC
            *(psp - 3) = 0xFFFFFFFD;                // Store LR
            *(psp - 4) = 0xFFFFFFFF;                // Store R12
            *(psp - 5) = 0xFFFFFFFF;                // Store R3
            *(psp - 6) = 0xFFFFFFFF;                // Store R2
            *(psp - 7) = 0xFFFFFFFF;                // Store R1
            *(psp - 8) = 0xFFFFFFFF;                // Store R0

            psp = psp - 0x08;                       // Update the PSP pointer to drop down 8 locations so POP can happen without issues

            __asm(" MSR PSP, R0");                  // Load the PSP into the register
            __asm(" MOVW R0, #0xFFFD");             // Load the lower half-word
            __asm(" MOVT R0, #0xFFFF");             // Load the upper half-word
            __asm(" MOV LR, R0");                   // Move the value into the Link Register so the processor auto POP everything
            __asm(" BX      LR");                   // Branch back
            break;
        }

        case STATE_READY:
        {
            __asm(" MRS     R1, PSP");              // Load the PSP into a local register
            __asm(" SUBS    R1, #0x24");            // Go down 9 registers and pop from there
            __asm(" LDMIA   R1!, {R4-R11, LR}");    // Load registers R4-R11 from the stack
            __asm(" BX      LR");                   // Branch back
            break;
        }

        default:
        {
            break;
        }
    }
    // if(getPendSVFlags())    clearPendSVFlags();
}

// REQUIRED: in preemptive code, add code to handle synchronization primitives
void svCallIsr(void)
{
    enablePendSV();
}
