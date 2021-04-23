;-------------------------------------------------------------------------------
; In this example a number is displayed on 7seg display every time
; one button is pressed. Button press is detected using interrupt.
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
            
;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
            .ref	WriteLed				; display function
;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP         ; Initialize stackpointer
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer

setup:		bis.b	#BIT7, &P3DIR			; configure P3.7
			bis.b	#BIT7, &P3OUT
			bis.b	#BIT3, &P4DIR			; configure P4.3
			bis.b	#BIT3, &P4OUT
			bis.b	#BIT6, &P2DIR			; configure P2.6
			bis.b	#BIT6, &P2OUT
			bis.b	#BIT1, &P8DIR			; configure P8.1
			bis.b	#BIT1, &P8OUT
			bis.b	#BIT3, &P2DIR			; configure P2.3
			bis.b	#BIT3, &P2OUT
			bis.b	#BIT0, &P4DIR			; configure P4.0
			bis.b	#BIT0, &P4OUT
			bis.b	#BIT2, &P8DIR			; configure P8.2
			bis.b	#BIT2, &P8OUT

			bis.b	#BIT0, &P7DIR
			bic.b	#BIT0, &P7OUT			; enable SEL1 signal


			bic.b	#BIT5, &P1DIR			; configure P1.5 as in
			bis.b	#BIT5, &P1REN			; enable pull up/down
			bis.b	#BIT5, &P1OUT			; configure pull up
			bis.b	#BIT5, &P1IES			; interrupt on falling edge
			bic.b	#BIT5, &P1IFG			; clear interrupt flag
			bis.b	#BIT5, &P1IE			; enable interrupt on P1.5
			nop
			bis.b	#GIE, SR				; enable maskable interrupts
			nop								; this removes the warning
;-------------------------------------------------------------------------------
; Main loop here
;-------------------------------------------------------------------------------

            jmp     $                       ; dummy loop

; P1 interrupt routine
            .text
P1ISR       mov.w   #0xfff,R9               ; debounce period
wait:       dec     R9                      ; debounce
            jnz     wait
            bit.b   #BIT5,&P1IFG            ; check if P1.5 triggered isr
            jz      isr_exit
            bit.b   #BIT5,&P1IN             ; check if button is still pressed
            jnz     isr_exit                ; if it is not pressed, exit isr
            call	#WriteLed	            ; toggle P2.4
isr_exit:   bic.b   #BIT5,&P1IFG            ; clear P1.5 interrupt flag
            reti
                                            

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
            
            .sect   ".int47"                  ; setup P1 ISR
            .short  P1ISR
