
    .def switchToPSP
    .def getPSP
    .def getMSP
    .def getFaultFlags
    .def disablePrivilegedMode
    .def enablePrivilegedMode

; Set the ASP bit in the CONTROL register to use PSP for thread code
switchToPSP:
    MRS R1, CONTROL     ; Read the current CONTROL register value
    ORR R1, R1, R0      ; Set ASP bit (bit 1) to enable PSP
    MSR CONTROL, R1     ; Write the updated value back to CONTROL register
    ISB                 ; Instruction barrier to ensure correct stack usage
    BX LR               ; Return

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
    ORR R0, R0, #1      ; Set the nPRIV bit (bit 0) to switch to unprivileged mode
    MSR CONTROL, R0     ; Write the updated value back to CONTROL register
    ISB                 ; Instruction Synchronization Barrier (ensure proper execution order)

enablePrivilegedMode:
    MRS R0, CONTROL     ; Read the current CONTROL register value
    BIC R0, R0, #0x01   ; Clear the unprivileged bit (bit 0)
    MSR CONTROL, R0     ; Write the updated value back to CONTROL register
    ISB                 ; Instruction Synchronization Barrier