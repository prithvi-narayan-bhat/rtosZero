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

#define     CURRENT_MUTEX       mutexes[tcb[taskCurrent].mutex]
#define     FIRST_MUTEX         mutexes[0]

#define     CURRENT_SEMAPHORE   semaphores[tcb[taskCurrent].semaphore]

#define     YIELD               0x00                // SVC number for YIELD
#define     SLEEP               0x01                // SVC number for sleep
#define     LOCK                0x02                // SVC number for mutex lock
#define     UNLOCK              0x03                // SVC number for mutex unlock
#define     WAIT                0x04                // SVC number for semaphore wait
#define     POST                0x05                // SVC number for semaphore post
#define     STOP                0x06                // SVC number to stop or kill a thread using it's PID
#define     RESTART             0x07                // SVC number to restart a thread using it's PID
#define     REBOOT              0x08                // SVC number to reset the system
#define     PS                  0x09                // SVC number for the PS command
#define     SCHED               0x11                // SVC number to change scheduler mode
#define     PREEMPT             0x12                // SVC number to change preemption mode
#define     PID                 0x13                // SVC number to get PID from given string
#define     PKILL               0x14                // SVC number to stop or kill a thread using it's name
#define     RUN                 0x15                // SVC number to restart a thread using it's name
#define     IPCS                0x16                // SVC number to get the status of IPC mechanisms
#define     SETPRIORITY         0x17                // SVC number to update the priority of a thread
#define     PRIORITY            0x18                // SVC number to update the priority inheritance state

mutex mutexes[MAX_MUTEXES];                         // Instantiate mutex globally
semaphore semaphores[MAX_SEMAPHORES];               // Instantiate mutex globally

// task states
#define STATE_INVALID           0                   // no task
#define STATE_STOPPED           1                   // stopped, can be resumed
#define STATE_UNRUN             2                   // task has never been run
#define STATE_READY             3                   // has run, can resume at any time
#define STATE_DELAYED           4                   // has run, but now awaiting timer
#define STATE_BLOCKED_MUTEX     5                   // has run, but now blocked by semaphore
#define STATE_BLOCKED_SEMAPHORE 6                   // has run, but now blocked by semaphore

// PS
uint8_t activeFillIndex_g = 0;
uint32_t lastTimeStamp_g = 0;
uint16_t oneSecondLoad_g = 1000;

// Faults
uint32_t pidExtern_g = 0;

// task
uint8_t taskCurrent = 0;                            // index of last dispatched task
uint8_t taskCount = 0;                              // total number of valid tasks

// control
bool priorityScheduler = true;                      // priority (true) or round-robin (false)
bool priorityInheritance = false;                   // priority inheritance for mutexes
bool preemption = false;                            // preemption (true) or cooperative (false)

// Task Control Block
#define NUM_PRIORITIES   8
struct _tcb
{
    void *pid;                                      // used to uniquely identify thread (add of task fn)
    void *spInit;                                   // original top of stack
    void *sp;                                       // current stack pointer
    uint32_t ticks;                                 // ticks until sleep complete
    uint32_t scheduledCount;                        // To keep track of how many times the task was scheduled
    uint32_t runTime[2];                            // To hold the runTime values

    uint8_t state;                                  // see STATE_ values above
    uint8_t priority;                               // 0=highest
    uint8_t currentPriority;                        // 0=highest (needed for pi)
    uint8_t runInstances;                           // Number of instances task was scheduled
    uint8_t srd[NUM_SRAM_REGIONS];                  // MPU subregion disable bits
    char name[16];                                  // name of task used in ps command
    uint8_t mutex;                                  // index of the mutex in use or blocking the thread
    uint8_t semaphore;                              // index of the semaphore that is blocking the thread
} tcb[MAX_TASKS];

/**
*      @brief Function to initialize the mutex structure
*      @param mutex to be initialized
*      @return true if initialization is successful
*      @return false if mutex count exceeds maximum
**/
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

/**
 *      @brief Function to initialise the semaphore structure variables
 *      @param semaphore to be initialise
 *      @param count of the semaphore
 *      @return true if successful
 *      @return false if unsuccessful
 **/
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
*      @brief Function to initialise timer module
**/
void initTimer(void)
{
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R0;    // Enable and provide clock to the timer
    _delay_cycles(3);                               // Delay for sync

    WTIMER0_CTL_R       &= ~TIMER_CTL_TAEN;         // Disable timer before configuring
    WTIMER0_CFG_R       = TIMER_CFG_32_BIT_TIMER;   // Select 32 bit wide counter
    WTIMER0_TAMR_R      |= TIMER_TAMR_TACDIR;       // Direction = Up-counter
    WTIMER0_TAV_R       = 0;
}

/**
 *      @brief Function to initialise the Task Control Block before starting any threads
 **/
void initRtos(void)
{
    uint8_t i;

    initSysTick();                              // Initialise system ticks
    initTimer();                                // Initialise timer module

    taskCount = 0;                              // No tasks running

    for (i = 0; i < MAX_TASKS; i++)             // Clear out tcb records
    {
        tcb[i].state = STATE_INVALID;
        tcb[i].pid = 0;
    }
}

/**
*      @brief Priority Task Scheduler with round robin for tasks with same priority
*      @return uint8_t task to be executed
**/
uint8_t rtosScheduler(void)
{
    // Use priority scheduler
    if (priorityScheduler)
    {
        uint8_t currentHighestPriority = 0xFF;                                          // Arbitrarily high value
        uint8_t highestPriorityTask;
        uint8_t taskP;

        for (taskP = 0; taskP < MAX_TASKS; taskP++)                                     // Iterate through all tasks
        {
            if (tcb[taskP].state == STATE_READY || tcb[taskP].state == STATE_UNRUN)     // Find READY and UNRUN tasks
            {
                if (tcb[taskP].currentPriority < currentHighestPriority)                // Find the priority
                {
                    currentHighestPriority = tcb[taskP].currentPriority;                // Update the current highest priority
                    highestPriorityTask = taskP;                                        // Update the current task
                }

                else if (tcb[taskP].currentPriority == currentHighestPriority)          // If there are two ready tasks with same priority
                {
                    // Find the one that was scheduled fewer times
                    if (tcb[taskP].scheduledCount < tcb[highestPriorityTask].scheduledCount)
                    {
                        highestPriorityTask = taskP;
                    }
                }
            }
        }
        taskCurrent = highestPriorityTask;

        tcb[taskCurrent].scheduledCount++;                                              // Increment the schedule count

        return taskCurrent;                                                             // Return the task to be updated
    }

    // Use round-robin scheduler
    else
    {
        bool ok = false;
        static uint8_t task = 0xFF;                                                     // Arbitrarily high value

        while (!ok)                                                                     // Iterate over all tasks starting from last task
        {
            task++;
            if (task >= MAX_TASKS)      task = 0;                                       // Roll over task count
            ok = (tcb[task].state == STATE_READY || tcb[task].state == STATE_UNRUN);    // Schedule READY or UNRUN task
        }

        taskCurrent = task;                                                             // Update the current task
        tcb[taskCurrent].scheduledCount++;                                              // Increment the schedule count

        return taskCurrent;                                                             // Return the task count
    }
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
    pidExtern_g = (uint32_t)taskPID;          // Expose it to the outside world
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
            tcb[i].currentPriority  = priority;                             // Store the requested PID

            generateSrdMasks(ptr, stackBytes, tcb[i].srd);                  // Store SRD masks in the TCB

            taskCount++;                                                    // Increment record of task count
            ok = true;
        }
    }
    return ok;
}

/**
 *      @brief Function to restart a thread
 *      @param fn pointer to the function to be restarted
 **/
void restartThread(_fn fn)
{
    __asm(" SVC #0x07");                                    // Trigger a Service call
}

/**
*      @brief Function to stop a thread
*      @param fn to be stopped
**/
void stopThread(_fn fn)
{
    __asm(" SVC #0x06");                                    // Trigger a Service call
}

/**
*      @brief Function to set the Thread Priority
*      @param fn pointer to the thread to be updated
*      @param priority to be set for the thread
**/
void setThreadPriority(_fn fn, uint8_t priority)
{
    __asm(" SVC #0x17");                                    // Trigger a Service call
}

/**
 *      @brief Function to yield execution back to scheduler using pendSv
 **/
void yield(void)
{
    __asm(" SVC #0x00");                                    // Trigger a Service call
}

/**
*      @brief Function to yield control back to the scheduler for the duration specified
*      @param tick sleep time in ms
**/
void sleep(uint32_t tick)
{
    __asm(" SVC #0x01");                                    // Trigger a Service call
}

/**
 *      @brief Function to lock a mutex using pendSv
 *      @param mutex mutex number
 **/
void lock(int8_t mutex)
{
    __asm(" SVC #0x02");                                    // Trigger a Service call
}

/**
 *      @brief Function to unlock a mutex using pendSv
 *      @param mutex mutex number
 **/
void unlock(int8_t mutex)
{
    __asm(" SVC #0x03");                                    // Trigger a Service call
}

/**
 *      @brief Function to wait for a semaphore pendSv
 *      @param semaphore semaphore number
 **/
void wait(int8_t semaphore)
{
    __asm(" SVC #0x04");                                    // Trigger a Service call
}

/**
 *      @brief Function to signal a semaphore is available using pendsv
 *      @param semaphore semaphore number
 **/
void post(int8_t semaphore)
{
    __asm(" SVC #0x05");                                    // Trigger a Service call
}

/**
*      @brief Function to decrement the tick count every 1ms
**/
void systickIsr(void)
{
    uint8_t i;
    for (i = 0; i < taskCount; i++)
    {
        if (tcb[i].state == STATE_DELAYED)                  // Decrement tick for threads marked "DELAYED"
        {
            tcb[i].ticks--;                                 // Decrement the ticks count
            if (tcb[i].ticks == 0)                          // Update state to ready if ticks run out
            {
                tcb[i].state = STATE_READY;
            }
        }
    }

    if (preemption)     enablePendSV();

    if (oneSecondLoad_g)
    {
        oneSecondLoad_g--;
    }

    else if (!oneSecondLoad_g)                              // One second has elapsed
    {
        oneSecondLoad_g = 999;                              // Reload the value
        for (i = 0; i < taskCount; i++)
        {
            tcb[i].runTime[!activeFillIndex_g] = 0;         // Zero out the values before accumulating new ones
        }
        activeFillIndex_g = !activeFillIndex_g;             // Start saving time in the other index
    }
}

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
    __asm(" MRS     R0, PSP");                              // Load the PSP into a local register in the stack frame
    __asm(" STMDB   R0, {R4-R11, LR}");                     // Store registers R4-R11 and LR in the stack frame

    tcb[taskCurrent].sp = (void *)getPSP();                 // Store the PSP to the sp of the current task
    tcb[taskCurrent].runTime[activeFillIndex_g] = WTIMER0_TAV_R - lastTimeStamp_g;

    rtosScheduler();                                        // Invoke RTOS scheduler, get next task

    pidExtern_g = (uint32_t)tcb[taskCurrent].pid;
    applySrdRules(tcb[taskCurrent].srd);                    // Apply the SRD rules specific to the first thread
    loadPSP((uint32_t)tcb[taskCurrent].sp);                 // Load the new PSP and execute

    switch (tcb[taskCurrent].state)
    {
        case STATE_UNRUN:
        {
            tcb[taskCurrent].state = STATE_READY;           // Update old state to be ready

            // Create a stack frame to trick the processor into thinking this thread was previously run
            uint32_t *psp = (uint32_t *)tcb[taskCurrent].sp; // Get the stack pointer
            *(psp - 1) = 0x01000000;                        // Load the Thumb bit in the xPSR or things go south
            *(psp - 2) = (uint32_t)tcb[taskCurrent].pid;    // Store PC
            *(psp - 3) = 0xFFFFFFFD;                        // Store LR
            *(psp - 4) = 0xFFFFFFFF;                        // Store R12
            *(psp - 5) = 0xFFFFFFFF;                        // Store R3
            *(psp - 6) = 0xFFFFFFFF;                        // Store R2
            *(psp - 7) = 0xFFFFFFFF;                        // Store R1
            *(psp - 8) = 0xFFFFFFFF;                        // Store R0

            psp = psp - 0x08;                               // Update the PSP pointer to drop down 8 locations to POP

            __asm(" MSR PSP, R0");                          // Load the PSP into the register
            __asm(" MOVW R0, #0xFFFD");                     // Load the lower half-word
            __asm(" MOVT R0, #0xFFFF");                     // Load the upper half-word
            __asm(" MOV LR, R0");                           // Move the value into the Link Register so the processor auto POP everything

            lastTimeStamp_g = WTIMER0_TAV_R;
            WTIMER0_CTL_R |= TIMER_CTL_TAEN;                // Enable timer before branching out to thread

            __asm(" BX      LR");                           // Branch back
            break;
        }

        case STATE_READY:
        {
            __asm(" MRS     R1, PSP");                      // Load the PSP into a local register
            __asm(" SUBS    R1, #0x24");                    // Go down 9 registers and pop from there
            __asm(" LDMIA   R1!, {R4-R11, LR}");            // Load registers R4-R11 from the stack

            lastTimeStamp_g = WTIMER0_TAV_R;
            WTIMER0_CTL_R |= TIMER_CTL_TAEN;                // Enable timer before branching out to thread

            __asm(" BX      LR");                           // Branch back
            break;
        }
    }
}

/**
 *      @brief Service Call (SVC) handler for handling SVC requests from tasks
 *          This function is called in response to SVC instructions triggered by tasks
 *          It processes different SVC priority values and performs corresponding actions
 *          Currently supports yield and sleep
 **/
void svCallIsr(void)
{
    uint8_t i, j;
    char dest[20];
    bool exists = false;
    uint32_t svcAction = getSvcPriority();                                                  // Get the action value from the SVC request

    switch (svcAction)                                                                      // Check the action value to determine the action to take
    {
        case YIELD:
        {
            enablePendSV();
            break;
        }

        case SLEEP:                                                                         // Cause function to sleep
        {
            tcb[taskCurrent].state = STATE_DELAYED;                                         // Set state to Delayed in the Task Control Block
            tcb[taskCurrent].ticks = getArgs();                                             // Get the Ticks from R0

            enablePendSV();                                                                 // Enable PendSV to perform a context switch

            break;
        }

        case LOCK:
        {
            tcb[taskCurrent].mutex = (uint8_t)getArgs();                                    // Get the mutex value

            if (!CURRENT_MUTEX.lock)                                                        // Mutex is free
            {
                CURRENT_MUTEX.lockedBy = taskCurrent;                                       // Say who's locking it
                CURRENT_MUTEX.lock = true;
            }

            // Priority Inheritance
            else if (priorityInheritance && (tcb[CURRENT_MUTEX.lockedBy].currentPriority < tcb[taskCurrent].currentPriority))
            {
                // Elevate priority of the task holding the resource to that of one requesting it
                tcb[CURRENT_MUTEX.lockedBy].currentPriority = tcb[taskCurrent].currentPriority;
            }

            // Priority inheritance is disabled. Add process to queue
            else if (CURRENT_MUTEX.queueSize < MAX_MUTEX_QUEUE_SIZE)                        // Add task to queue only if mutex queue is empty
            {
                CURRENT_MUTEX.processQueue[CURRENT_MUTEX.queueSize++] = taskCurrent;
                tcb[taskCurrent].state = STATE_BLOCKED_MUTEX;                               // Set state to Delayed in the Task Control Block
            }

            enablePendSV();                                                                 // Enable PendSV to perform a context switch

            break;
        }

        case UNLOCK:
        {
            tcb[taskCurrent].mutex = (uint8_t)getArgs();                                    // Get the mutex value

            if (CURRENT_MUTEX.lockedBy == taskCurrent)
            {
                if (CURRENT_MUTEX.queueSize)                                                // Can have a max of 2 tasks in the queue
                {
                    tcb[CURRENT_MUTEX.processQueue[0]].state = STATE_READY;                 // Ready the oldest waiting task to ready
                    CURRENT_MUTEX.lockedBy = CURRENT_MUTEX.processQueue[0];                 // Update the ID of the task locking the resource

                    if (CURRENT_MUTEX.queueSize == 2)                                       // Can have a max of 2 tasks in the queue
                    {
                        CURRENT_MUTEX.processQueue[0] = CURRENT_MUTEX.processQueue[1];      // Shift the queue up
                    }
                    CURRENT_MUTEX.queueSize--;                                              // Decrement count of waiting processes
                }

                else
                {
                    CURRENT_MUTEX.lock = false;                                             // Indicate that mutex is available
                }

                // Revert to the original priority
                if (priorityInheritance && tcb[CURRENT_MUTEX.lockedBy].currentPriority != tcb[CURRENT_MUTEX.lockedBy].priority)
                {
                    tcb[CURRENT_MUTEX.lockedBy].currentPriority = tcb[CURRENT_MUTEX.lockedBy].priority;
                }
                enablePendSV();                                                             // Enable PendSV to perform a context switch
            }
            break;
        }

        case WAIT:
        {
            tcb[taskCurrent].semaphore = (uint8_t)getArgs();                                // Get semaphore value

            if (CURRENT_SEMAPHORE.count >= 1)                                               // If semaphore value is greater than zero, decrements
            {
                CURRENT_SEMAPHORE.count--;
            }

            else if (CURRENT_SEMAPHORE.queueSize < MAX_SEMAPHORE_QUEUE_SIZE)
            {
                for (i = 0; i < CURRENT_SEMAPHORE.queueSize; i++)
                {
                    if (CURRENT_SEMAPHORE.processQueue[i] == taskCurrent)                   // Process is already in queue
                    {
                        exists = true;                                                      // Set a flag and break
                        break;
                    }
                }
                if (!exists)                                                                // Add to queue if doesn't exist
                {
                    CURRENT_SEMAPHORE.processQueue[CURRENT_SEMAPHORE.queueSize++] = taskCurrent;
                    tcb[taskCurrent].state = STATE_BLOCKED_SEMAPHORE;                       // Set state to Delayed in the Task Control Block
                }
            }

            enablePendSV();                                                                 // Enable PendSV to perform a context switch

            break;
        }

        case POST:
        {
            tcb[taskCurrent].semaphore = (uint8_t)getArgs();                                // Get semaphore value

            if (CURRENT_SEMAPHORE.queueSize)                                                // Someone is waiting the queue
            {
                tcb[CURRENT_SEMAPHORE.processQueue[0]].state = STATE_READY;                 // Update state

                if (CURRENT_SEMAPHORE.queueSize == 2)
                {
                    CURRENT_SEMAPHORE.processQueue[0] = CURRENT_SEMAPHORE.processQueue[1];  // Shift up the queue
                    CURRENT_SEMAPHORE.queueSize--;                                          // Update queue size
                }
            }

            else
            {
                CURRENT_SEMAPHORE.count++;                                                  // Update that there's one more spot in the semaphore
            }

            enablePendSV();                                                                 // Enable PendSV to perform a context switch

            break;
        }

        case STOP:
        {
            uint32_t pidToStop = (uint32_t)getArgs();                                       // Get the task to be stopped

            for (i = 0; i < MAX_TASKS; i++)
            {
                if ((uint32_t)tcb[i].pid == pidToStop)
                {
                    // Remove task from Mutex queue
                    if (tcb[i].state == STATE_BLOCKED_MUTEX)                                // Task is waiting the queue
                    {
                        for (j = 0; j < mutexes[tcb[i].mutex].queueSize; j++)
                        {
                            if (mutexes[tcb[i].mutex].processQueue[j] == taskCurrent)       // Find task
                            {
                                if ((i + 1) < mutexes[tcb[i].mutex].queueSize)              // Move lower task to current tasks position
                                {
                                    mutexes[tcb[i].mutex].processQueue[j] = mutexes[tcb[i].mutex].processQueue[j + 1];
                                    mutexes[tcb[i].mutex].queueSize--;                      // Decrement queue size
                                }
                            }
                        }
                    }

                    // Remove task from Semaphore queue
                    if (tcb[i].state == STATE_BLOCKED_SEMAPHORE)                            // Task is waiting the queue
                    {
                        for (j = 0; j < semaphores[tcb[i].semaphore].queueSize; j++)
                        {
                            // Find task
                            if (semaphores[tcb[i].semaphore].processQueue[j] == taskCurrent)
                            {
                                if ((i + 1) < semaphores[tcb[i].semaphore].queueSize)       // Move lower task to current tasks position
                                {
                                    semaphores[tcb[i].semaphore].processQueue[j] = semaphores[tcb[i].semaphore].processQueue[j + 1];
                                    semaphores[tcb[i].semaphore].queueSize--;               // Decrement queue size
                                }
                            }
                        }
                    }

                    tcb[i].mutex      = 0;                                                  // Clear values from the TCB
                    tcb[i].semaphore  = 0;                                                  // Clear values from the TCB
                    tcb[i].ticks      = 0;                                                  // Clear values from the TCB
                    tcb[i].state      = STATE_STOPPED;                                      // Mark the state of the thread as stopped

                    break;                                                                  // Break out of the loop
                }
            }

            print((void *)&pidToStop, "Stopped", INT);
            enablePendSV();                                                                 // Enable PendSV to perform a context switch
            break;
        }

        case RESTART:
        {
            uint32_t pidToStart = (uint32_t)getArgs();                                      // Get the task to be restarted

            for (i = 0; i < MAX_TASKS; i++)                                                 // Iterate over all tasks
            {
                if ((uint32_t)tcb[i].pid == pidToStart)                                     // Find the task to be started
                {
                    tcb[i].state = STATE_READY;                                             // Update the state to ready
                    break;
                }
            }
            print((void *)&pidToStart, "Restarted", INT);
            enablePendSV();                                                                 // Enable PendSV
            break;
        }

        case REBOOT:
        {
            putsUart0("Rebooting now\r\n");
            NVIC_APINT_R = (NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ);                   // Reset System
            break;
        }

        case PS:
        {
            putsUart0("Task\t PID\t CPU\t Name\r\n");
            for (i = 0; i < taskCount; i++)
            {
                putsUart0(itoa(i, dest));
                putsUart0("\t ");
                putsUart0(itoa((uint32_t)tcb[i].pid, dest));
                putsUart0("\t ");
                putsUart0(itoa(((tcb[i].runTime[!activeFillIndex_g]) / 400), dest));
                putsUart0("%\t ");
                putsUart0(tcb[i].name);
                putsUart0("\r\n");
            }
            putsUart0("\r\n");
            break;
        }

        case SCHED:
        {
            priorityScheduler = getArgs();
            if (priorityScheduler)      putsUart0("Scheduler Mode: Priority\r\n");
            else                        putsUart0("Scheduler Mode: Round-Robin\r\n");

            enablePendSV();

            break;
        }

        case PREEMPT:
        {
            preemption = getArgs();
            if (priorityScheduler)      putsUart0("Preemption Mode: On\r\n");
            else                        putsUart0("Preemption Mode: Off\r\n");

            enablePendSV();

            break;
        }

        case PID:
        {
            char *functionName = (char *)getArgs();

            for (i = 0; i < MAX_TASKS; i++)
            {
                if (!(strcmp(tcb[i].name, functionName)))
                {
                    print((void *)&tcb[i].pid, "PID", INT);
                    break;
                }
            }
            break;
        }

        case PKILL:
        {
            char *funcToStop = (char *)getArgs();                                           // Get the task to be stopped

            for (i = 0; i < MAX_TASKS; i++)                                                 // Iterate over all tasks
            {
                if (!(strcmp(tcb[i].name, funcToStop)))                                     // Match the function name in the TCB
                {
                    // Remove task from Mutex queue
                    if (tcb[i].state == STATE_BLOCKED_MUTEX)                                // Task is waiting the queue
                    {
                        for (j = 0; j < mutexes[tcb[i].mutex].queueSize; j++)
                        {
                            if (mutexes[tcb[i].mutex].processQueue[j] == taskCurrent)       // Find task
                            {
                                if ((i + 1) < mutexes[tcb[i].mutex].queueSize)              // Move lower task to current tasks position
                                {
                                    mutexes[tcb[i].mutex].processQueue[j] = mutexes[tcb[i].mutex].processQueue[j + 1];
                                    mutexes[tcb[i].mutex].queueSize--;                      // Decrement queue size
                                }
                            }
                        }
                    }

                    // Remove task from Semaphore queue
                    if (tcb[i].state == STATE_BLOCKED_SEMAPHORE)                            // Task is waiting the queue
                    {
                        for (j = 0; j < semaphores[tcb[i].semaphore].queueSize; j++)
                        {
                            // Find task
                            if (semaphores[tcb[i].semaphore].processQueue[j] == taskCurrent)
                            {
                                if ((i + 1) < semaphores[tcb[i].semaphore].queueSize)       // Move lower task to current tasks position
                                {
                                    semaphores[tcb[i].semaphore].processQueue[j] = semaphores[tcb[i].semaphore].processQueue[j + 1];
                                    semaphores[tcb[i].semaphore].queueSize--;               // Decrement queue size
                                }
                            }
                        }
                    }

                    tcb[i].mutex      = 0;                                                  // Clear values from the TCB
                    tcb[i].semaphore  = 0;                                                  // Clear values from the TCB
                    tcb[i].ticks      = 0;                                                  // Clear values from the TCB
                    tcb[i].state      = STATE_STOPPED;                                      // Mark the state of the thread as stopped

                    break;                                                                  // Break out of the loop
                }
            }

            print((void *)funcToStop, "killed", CHAR);
            enablePendSV();                                                                 // Enable PendSV to perform a context switch
            break;
        }

        case RUN:
        {
            char  *pidToStart = (char *)getArgs();                                          // Get the task to be restarted

            for (i = 0; i < MAX_TASKS; i++)                                                 // Iterate over all tasks
            {
                if (!(strcmp(tcb[i].name, pidToStart)))                                     // Find the task to be started
                {
                    tcb[i].state = STATE_READY;                                             // Update the state to ready
                }
            }
            print((void *)pidToStart, "Running ", CHAR);
            enablePendSV();                                                                 // Enable PendSV
            break;
        }

        case IPCS:
        {
            char dest[20];
            putsUart0("----Semaphore Arrays----\r\n");
            for (i = 0; i < MAX_SEMAPHORES; i++)
            {
                putsUart0("\r\nSemaphore: ");
                putsUart0(itoa(i, dest));
                putsUart0("\r\n\tCount:        ");
                putsUart0(itoa(semaphores[i].count, dest));
                putsUart0("\r\n\tQueue Size:   ");
                putsUart0(itoa(semaphores[i].queueSize, dest));
                putsUart0("\r\n\tQueued PIDs:  ");
                for (j = 0; j < semaphores[i].queueSize; j++)
                {
                    putsUart0(itoa((uint32_t)tcb[semaphores[i].processQueue[j]].pid, dest));
                    putsUart0(" ");
                }
                putsUart0("\r\n");
            }

            putsUart0("\r\n\r\n----Mutex Arrays----\r\n");
            for (i = 0; i < MAX_MUTEXES; i++)
            {
                putsUart0("\r\nMutex: ");
                putsUart0(itoa(i, dest));
                putsUart0("\r\n\tLocked By:    ");
                putsUart0(itoa(mutexes[i].lockedBy, dest));
                putsUart0("\r\n\tQueue Size:   ");
                putsUart0(itoa(mutexes[i].queueSize, dest));
                putsUart0("\r\n\tQueued tasks: ");
                for (j = 0; j < mutexes[i].queueSize; j++)
                {
                    putsUart0(itoa((uint32_t)tcb[mutexes[i].processQueue[j]].pid, dest));
                    putsUart0(" ");
                }
                putsUart0("\r\n");
            }

            break;
        }

        case SETPRIORITY:
        {
            uint32_t pid = (uint32_t)getArgs();
            uint32_t *psp = (uint32_t *)getPSP();
            uint32_t priority = *(psp + 1);

            for (i = 0; i < MAX_TASKS; i++)
            {
                if ((uint32_t)tcb[i].pid == pid)
                {
                    tcb[i].priority = priority;
                    tcb[i].currentPriority = priority;
                    break;
                }
            }

            putsUart0("Priority updated\r\n");

            enablePendSV();
            break;
        }

        case PRIORITY:
        {
            priorityInheritance = getArgs();

            if (priorityInheritance)    putsUart0("Priority Inheritance mode: On\r\n");
            else                        putsUart0("Priority Inheritance mode: Off\r\n");

            enablePendSV();

            break;
        }
    }
}
