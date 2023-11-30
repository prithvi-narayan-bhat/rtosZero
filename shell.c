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

    char dest[20];

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

            else IS_COMMAND("sched", 2)
            {
                char *scheduleState = getFieldString(&shellData, 1);        // Get arguments
                priority(toBool(scheduleState));                            // Invoke function
                putsUart0("\r\n\r\n");
                yield();
            }

            else IS_COMMAND("preempt", 2)
            {
                char *preemptionState = getFieldString(&shellData, 1);      // Get arguments
                preempt(toBool(preemptionState));                           // Invoke function
                putsUart0("\r\n\r\n");
                yield();
            }

            else IS_COMMAND("pidof", 2)
            {
                char *procName = getFieldString(&shellData, 1);             // Get arguments
                uint32_t pid;
                pidof(procName, (void *)&pid);                              // Invoke function
                putsUart0(itoa(pid, dest));
                putsUart0("\r\n\r\n");
                yield();
            }

            else IS_COMMAND("kill", 2)
            {
                uint32_t pid = (uint32_t)getFieldInteger(&shellData, 1);    // Get arguments
                kill(pid);                                                  // Invoke function
                putsUart0("\r\n\r\n");
                yield();
            }

            else IS_COMMAND("pkill", 2)
            {
                char *procName = getFieldString(&shellData, 1);             // Get arguments
                Pkill(procName);                                            // Invoke function
                putsUart0("\r\n\r\n");
                yield();
            }

            else IS_COMMAND("run", 2)
            {
                char *procName = getFieldString(&shellData, 1);             // Get arguments
                run(procName);                                              // Invoke function
                putsUart0("\r\n\r\n");
                yield();
            }

            else IS_COMMAND("ipcs", 1)
            {
                mutexInfo_t mutexInfo[1];
                semaphoreInfo_t semaphoreInfo[3];

                ipcs((void *)mutexInfo, (void *)semaphoreInfo);                                                     // Invoke function

                uint8_t i, j;
                putsUart0("----Semaphore Arrays----\r\n");
                for (i = 0; i < 3; i++)
                {
                    putsUart0("\r\n-----------------------------------");
                    putsUart0("\r\n Semaphore    | ");
                    putsUart0(itoa(i, dest));
                    putsUart0("\r\n--------------|--------------------");
                    putsUart0("\r\n Count        | ");
                    putsUart0(itoa(semaphoreInfo[i].count, dest));
                    putsUart0("\r\n Queue Size   | ");
                    putsUart0(itoa(semaphoreInfo[i].queueSize, dest));
                    putsUart0("\r\n Queued PIDs  | ");
                    for (j = 0; j < semaphoreInfo[i].queueSize; j++)
                    {
                        putsUart0(itoa((uint32_t)semaphoreInfo[i].processQueue[j], dest));
                        putsUart0(" ");
                        putsUart0(semaphoreInfo[i].processName[j]);
                        putsUart0(" ");
                    }
                    putsUart0("\r\n-----------------------------------\r\n");
                    putsUart0("\r\n");
                }

                putsUart0("\r\n\r\n----Mutex Arrays----\r\n");
                for (i = 0; i < 1; i++)
                {
                    putsUart0("\r\n-----------------------------------");
                    putsUart0("\r\n Mutex        | ");
                    putsUart0(itoa(i, dest));
                    putsUart0("\r\n--------------|--------------------");
                    putsUart0("\r\n Locked By    | ");
                    putsUart0(itoa(mutexInfo[i].lockedBy, dest));
                    putsUart0("    ");
                    putsUart0(mutexInfo[i].lockedByName);
                    putsUart0("\r\n Queue Size   | ");
                    putsUart0(itoa(mutexInfo[i].queueSize, dest));
                    putsUart0("\r\n Queued tasks | ");
                    for (j = 0; j < mutexInfo[i].queueSize; j++)
                    {
                        putsUart0(itoa((uint32_t)mutexInfo[i].processQueue[j], dest));
                        putsUart0(" ");
                        putsUart0(mutexInfo[i].processName[j]);
                        putsUart0(" ");
                    }
                    putsUart0("\r\n-----------------------------------\r\n");
                    putsUart0("\r\n");
                }

                putsUart0("\r\n\r\n");

                yield();
            }

            else IS_COMMAND("setpriority", 3)
            {
                uint32_t pid = (uint32_t)getFieldInteger(&shellData, 1);    // Get arguments
                uint32_t priority = (uint32_t)getFieldInteger(&shellData, 2); // Get arguments
                setThreadPriority((_fn)pid, priority);                      // Invoke function
                putsUart0("\r\n\r\n");
                yield();
            }

            else IS_COMMAND("inheritance", 2)
            {
                char *inheritanceState = getFieldString(&shellData, 1);     // Get arguments
                inheritance(toBool(inheritanceState));                      // Invoke function
                putsUart0("\r\n\r\n");
                yield();
            }

            else IS_COMMAND("ps", 1)
            {
                psInfo_t psInfo[12];

                uint8_t i;
                ps((void *)psInfo);                                         // Invoke function

                putsUart0("Task\t PID\t CPU\t Name\r\n");

                for (i = 0; i < 12; i++)
                {
                    if (!psInfo[i].pid)     break;

                    putsUart0(itoa(psInfo[i].task, dest));
                    putsUart0("\t ");

                    putsUart0(itoa((uint32_t)psInfo[i].pid, dest));
                    putsUart0("\t ");

                    putsUart0(insertDot(itoa(psInfo[i].cpuTime, dest)));
                    putsUart0("%\t ");

                    putsUart0(psInfo[i].name);
                    putsUart0("\r\n");
                }
                putsUart0("\r\n\r\n");
                yield();
            }

            else IS_COMMAND("help", 1)
            {
                putsUart0("\r\n\r\nUsage: command [args]\r\n\r\n");
                putsUart0("\tCommands   | Arguments\r\n");
                putsUart0("\t-----------|----------------\r\n");
                putsUart0("\treboot     |\r\n");
                putsUart0("\tipcs       |\r\n");
                putsUart0("\tps         |\r\n");
                putsUart0("\tsched      | [prio|rr]\r\n");
                putsUart0("\tpreempt    | [on|off]\r\n");
                putsUart0("\tinheritance| [on|off]\r\n");
                putsUart0("\tkill       | <pid>\r\n");
                putsUart0("\tpidof      | <function_name>\r\n");
                putsUart0("\tpkill      | <function_name>\r\n");
                putsUart0("\trun        | <function_name>\r\n");
                putsUart0("\tsetpriority| <pid> <priority>\r\n");
                putsUart0("\r\n\r\n");
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
