
    .def getPSP
    .def getMSP
    .def disablePrivilegedMode
    .def enablePrivilegedMode
    .def stageMethod
    .def setASP
    .def loadPSP

getPSP:
    MRS R0, PSP         ; Read the PSP register
    ISB                 ; Wait for sync
    BX LR               ; Return to the calling function

getMSP:
    MRS R0, MSP         ; Read the MSP register
    ISB                 ; Wait for sync
    BX LR               ; Return to the calling function

stageMethod:
    MSR PSP, R0         ; Load the address into PSP register
    ISB                 ; Instriction sync
    MRS R1, CONTROL     ; Read the current CONTROL register value
    ORR R1, R1, #0x02   ; Set ASP bit in CONTROL register
    MSR CONTROL, R1     ; Load the new CONTROL register value
    ISB                 ; Instruction sycn
    BX  LR              ; Return

loadPSP:
    MSR PSP, R0         ; Load the address into PSP register
    ISB                 ; Instriction sync
    BX LR;              ; Return

setASP:
    MRS R1, CONTROL     ; Read the current CONTROL register value
    ORR R1, R1, #0x02   ; Set the ASP bit
    MSR CONTROL, R1     ; Load the new CONTROL register value
    ISB                 ; Instruction sync
    BX LR               ; Return

disablePrivilegedMode:
    MRS R1, CONTROL     ; Read the current CONTROL register value
    ORR R1, R1, #0x01   ; Set the TMPL bit
    MSR CONTROL, R1     ; Write the updated value back to CONTROL register
    ISB                 ; Instruction Synchronization Barrier (ensure proper execution order)
    BX LR

enablePrivilegedMode:
    MRS R0, CONTROL     ; Read the current CONTROL register value
    BIC R0, R0, #0x03   ; Clear the ASP and TMPL bits
    MSR CONTROL, R0     ; Write the updated value back to CONTROL register
    ISB                 ; Instruction Synchronization Barrier
    BX LR