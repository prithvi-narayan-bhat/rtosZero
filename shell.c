#include "strings.h"
#include "commands.h"
#include "tm4c123gh6pm.h"
#include "shell.h"
#include "kernel.h"

#define IS_COMMAND(string, count)       if(isCommand(&shellData, string, count))
#define ASSERT(value)                   if(value >= 0)


void shell(void)
{
    shellData_t shellData = {0,};

    putsUart0("|============================================================|\r\n");
    putsUart0("|                         RTOSZer0                           |\r\n");
    putsUart0("|============================================================|\r\n\r\n");
    putsUart0("Type \"help\" for more..\r\n\r\n");

    while (true)
    {
        if (kbhitUart0())
        {
            getInputString(&shellData);                                     // Read user input
            parseInputString(&shellData);                                   // Parse user input

            IS_COMMAND("reboot", 1)
            {
                reboot();                                                   // Invoke function
                yield();
            }

            else IS_COMMAND("priority", 2)
            {
                char *scheduleState = getFieldString(&shellData, 1);        // Get arguments
                priority(toBool(scheduleState));                            // Invoke function
                yield();
            }

            else IS_COMMAND("preempt", 2)
            {
                char *preemptionState = getFieldString(&shellData, 1);      // Get arguments
                preempt(toBool(preemptionState));                           // Invoke function
                yield();
            }

            else IS_COMMAND("pidof", 2)
            {
                char *procName = getFieldString(&shellData, 1);             // Get arguments
                pidof(procName);                                            // Invoke function
                yield();
            }

            else IS_COMMAND("kill", 2)
            {
                uint32_t pid = (uint32_t)getFieldInteger(&shellData, 1);    // Get arguments
                kill(pid);                                                  // Invoke function
                yield();
            }

            else IS_COMMAND("pkill", 2)
            {
                char *procName = getFieldString(&shellData, 1);             // Get arguments
                Pkill(procName);                                            // Invoke function
                yield();
            }

            else IS_COMMAND("run", 2)
            {
                char *procName = getFieldString(&shellData, 1);             // Get arguments
                run(procName);                                              // Invoke function
                yield();
            }

            else IS_COMMAND("ipcs", 1)
            {
                ipcs();                                                     // Invoke function
                yield();
            }

            else IS_COMMAND("setpriority", 3)
            {
                uint32_t pid = (uint32_t)getFieldInteger(&shellData, 1);    // Get arguments
                uint32_t priority = (uint32_t)getFieldInteger(&shellData, 2); // Get arguments
                setThreadPriority((_fn)pid, priority);                      // Invoke function
            }


            else IS_COMMAND("ps", 1)
            {
                ps();                                                       // Invoke function
                yield();
            }

            else IS_COMMAND("help", 1)
            {
                putsUart0("\r\n\r\nUsage: command [args]\r\n");
                putsUart0("\r\n\tCommands Arguments\r\n");
                putsUart0("\treboot\r\n");
                putsUart0("\tipcs\r\n");
                putsUart0("\tsched       [prio|rr]\r\n");
                putsUart0("\tpreempt     [on|off]\r\n");
                putsUart0("\tkill        <pid>\r\n");
                putsUart0("\tpidof       <function_name>\r\n");
                putsUart0("\tPkill       <function_name>\r\n");
                putsUart0("\trun         <function_name>\r\n");
                putsUart0("\tsetpriority <pid> <priority>\r\n");
            }

            else
            {
                putsUart0("\r\n\r\nInvalid Command.\r\n");
                putsUart0("Type \"help\" for more..\r\n\r\n");
                yield();
            }
        }
        else
            yield();
    }
}
