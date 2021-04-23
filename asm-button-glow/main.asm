;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
            
;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP         ; Initialize stackpointer
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer

setup:		bic.b	#BIT4, &P2OUT			; clear P2.4
			bis.b	#BIT4, &P2DIR			; configure P2.4 as out

			bic.b	#BIT5, &P1DIR			; configure P1.5 as in
			bis.b	#BIT5, &P1REN			; enable pull up/down
			bis.b	#BIT5, &P1OUT			; configure pull up

;-------------------------------------------------------------------------------
; Main loop here
;-------------------------------------------------------------------------------
loop:		bit.b	#BIT5, &P1IN			; check state od P2.4
			jz		pressed					; if pressed ('0'), jump
			bic.b	#BIT4, &P2OUT			; clear P2.4, LED off
			jmp		loop					; return to loop
pressed:	bis.b	#BIT4, &P2OUT			; set P2.4, LED ON
			jmp		loop					; return to loop
			nop								; removes the warning


                                            

;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack
            
;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET
            
