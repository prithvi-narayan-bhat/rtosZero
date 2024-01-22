.thumb
@ .const

getPSP:
    MRS R0, PSP         @ Read the PSP register
    ISB                 @ Wait for sync
    BX LR               @ Return to the calling function

getMSP:
    MRS R0, MSP         @ Read the MSP register
    ISB                 @ Wait for sync
    BX LR               @ Return to the calling function

stageMethod:
    MSR PSP, R0         @ Load the address into PSP register
    ISB                 @ Instruction sync
    MRS R1, CONTROL     @ Read the current CONTROL register value
    @ Set ASP bit in CONTROL register
    MOV R2, #0x02
    ORR R1, R1, R2
    MSR CONTROL, R1     @ Load the new CONTROL register value
    ISB                 @ Instruction sycn
    BX  LR              @ Return

loadPSP:
    MSR PSP, R0         @ Load the address into PSP register
    ISB                 @ Instriction sync
    BX LR;              @ Return

setASP:
    MRS R1, CONTROL     @ Read the current CONTROL register value
    @ Set ASP bit in CONTROL register
    MOV R2, #0x02
    ORR R1, R1, R2
    MSR CONTROL, R1     @ Load the new CONTROL register value
    ISB                 @ Instruction sync
    BX LR               @ Return

disablePrivilegedMode:
    MRS R1, CONTROL     @ Read the current CONTROL register value
    @ Set ASP bit in CONTROL register
    MOV R2, #0x01
    ORR R1, R1, R2
    MSR CONTROL, R1     @ Write the updated value back to CONTROL register
    ISB                 @ Instruction Synchronization Barrier (ensure proper execution order)
    BX LR

enablePrivilegedMode:
    MRS R0, CONTROL     @ Read the current CONTROL register value
    @ Clear the ASP and TMPL bits
    MOVS R2, #0x03
    BIC R0, R0, R2
    MSR CONTROL, R0     @ Write the updated value back to CONTROL register
    ISB                 @ Instruction Synchronization Barrier
    BX LR

getSvcPriority:
    MRS R0, PSP         @ Load PSP into R0 to determine which function made the SV Call
    LDR R0, [R0, #24]   @ Load return address of that function
    @ LDRB R0, [R0, #-2]  @ Get the value of the argument from the location before the return address pointing to
    SUB R0, R1, #2            @ Subtract 2 to get the address before the return address
    LDRB R0, [R0]             @ Load a byte from the calculated address
    BX  LR              @ Return

getArgs:
    MRS R0, PSP         @ Load PSP into R0 to determine which function made the SV Call
    LDR R0, [R0]        @ Derefernce the value from PSP pointer
    BX  LR
