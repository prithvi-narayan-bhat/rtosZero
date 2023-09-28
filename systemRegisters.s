
    .def getPSP
    .def getMSP
    .def getFaultFlags
    .def disablePrivilegedMode
    .def enablePrivilegedMode

getPSP:
    MRS R0, PSP         ; Read the PSP register
    ISB                 ; Wait for sync
    BX LR               ; Return to the calling function

getMSP:
    MRS R0, MSP         ; Read the MSP register
    ISB                 ; Wait for sync
    BX LR               ; Return to the calling function

getFaultFlags:
    LDR R1, [R0]        ; Load value at the address
    BX LR               ; Return to calling function

disablePrivilegedMode:
    MRS R0, CONTROL     ; Read the current CONTROL register value
    ORR R0, R0, #0x03   ; Set the ASP and TMPL bits to switch to unprivileged mode
    MSR CONTROL, R0     ; Write the updated value back to CONTROL register
    ISB                 ; Instruction Synchronization Barrier (ensure proper execution order)

enablePrivilegedMode:
    MRS R0, CONTROL     ; Read the current CONTROL register value
    BIC R0, R0, #0x03   ; Clear the ASP and TMPL bits
    MSR CONTROL, R0     ; Write the updated value back to CONTROL register
    ISB                 ; Instruction Synchronization Barrier