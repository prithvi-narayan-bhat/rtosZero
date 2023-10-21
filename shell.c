#include "strings.h"
#include "commands.h"
#include "tm4c123gh6pm.h"
#include "shell.h"

#define IS_COMMAND(string, count)       if(isCommand(&shellData, string, count))
#define RESET                           (NVIC_APINT_R = (NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ))
#define ASSERT(value)                   if(value >= 0)


void shell(void)
{
    shellData_t shellData;

    if (kbhitUart0())
    {
        getInputString(&shellData);         // Read user input
        parseInputString(&shellData);       // Parse user input

        IS_COMMAND("ps", 1)     ps();                           // Invoke function
        IS_COMMAND("reboot", 1) RESET;                          // Reset System
        IS_COMMAND("ipcs", 1)   ipcs();                         // Invoke function
        IS_COMMAND("kill", 2)
        {
            uint32_t pid = (uint32_t)getFieldInteger(&shellData, 1);
            kill(pid);                      // Invoke function
            return;
        }

        IS_COMMAND("pkill", 2)              // Invoke function
        {
            char *procName = getFieldString(&shellData, 1);
            toLower(procName);
            Pkill(procName);
            return;
        }

        IS_COMMAND("preempt", 2)
        {
            char *preemptionState = getFieldString(&shellData, 1);
            preempt(toBool(preemptionState));
            return;
        }

        IS_COMMAND("sched", 2)
        {
            char *scheduleState = getFieldString(&shellData, 1);
            sched(toBool(scheduleState));
            return;
        }

        IS_COMMAND("pidof", 2)
        {
            char *procName = getFieldString(&shellData, 1);
            pidof(procName);
            return;
        }

        IS_COMMAND("run", 2)
        {
            char *procName = getFieldString(&shellData, 1);
            run(procName);
            return;
        }

        print((void *)"", "Invalid input", CHAR);
        return;
    }
    else
        return;
}
