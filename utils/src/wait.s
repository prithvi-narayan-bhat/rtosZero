# Wait Library
# Jason Losh, Prithvi Bhat

   @ .def waitMicrosecond
   @ .def _delay_cycles

.thumb

waitMicrosecond:
   WMS_LOOP0:  MOV  R1, #6          @ 1
   WMS_LOOP1:  SUB  R1, R1, #1      @ 6
               CMP  R1, #0
               BEQ  WMS_DONE1       @ 5+1*3
               NOP                  @ 5
               NOP                  @ 5
               B    WMS_LOOP1       @ 5*2
   WMS_DONE1:  SUB  R0, R0, #1      @ 1
               CMP  R0, #0
               BEQ  WMS_DONE0       @ 1
               NOP                  @ 1
               B    WMS_LOOP0       @ 1*2
   WMS_DONE0:  BX   LR              @ ---
                                    @ 40 clocks/us


_delay_cycles:
   DCS_LOOP0:  MOV R1, R0
               SUB R0, R0, #1
               CMP R0, #0
               BEQ DCS_EXIT
               NOP
               NOP
               B DCS_LOOP0
   DCS_EXIT:   BX LR
