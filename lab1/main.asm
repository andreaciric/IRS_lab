;-----------------------------------------------------------------------------------
;
; @file main.asm
; @brief In this example everytime S3 button is presssed value stored in register R10
; gets incremented and LED3 changes value. Everytime S4 button is presssed value
; stored in register R10 gets decremented and LED4 changes value.
; Value stored in R10 is diplayed on a sevenseg display.
; Button press is detected using interrupt.
;
; 4.1, 4.4 i 4.5
;
; @date 23.04.2021
; @author Andrea Ciric (andreaciric23@gmail.com)
;
; @version [1.0 - 04/2021] Initial version
;
;-----------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
            
;-----------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.

            .ref	WriteLed				; 4.1 in "WriteLed.asm" file
            .ref	LED_on_off				; 4.2 i 4.3 in "LED_on_off.asm" file
;-----------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-----------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer

			; 7seg display
setup:		bis.b	#0x48, &P2DIR			; P2.3 i P2.6 -> output
			bis.b	#0x48, &P2OUT			; P2.3 i P2.6 -> pull-up
			bis.b	#BIT7, &P3DIR			; P3.7 -> output
			bis.b	#BIT7, &P3OUT			; P3.7 -> pull-up
			bis.b	#0x09, &P4DIR			; P4.0 i P4.3 -> output
			bis.b	#0x09, &P4OUT			; P4.0 i P4.3 -> pull-up
			bis.b	#0x06, &P8DIR			; P8.1 i P8.2 -> output
			bis.b	#0x06, &P8OUT			; P8.1 i P8.2 -> pull-up

			bis.b	#BIT0, &P7DIR			; P7.0 <=> SEL1 -> output
			bic.b	#BIT0, &P7OUT			; SEL1 -> pull-down (npn)

			; Switch 3
			bic.b	#BIT4, &P1DIR			; P1.4 -> input
			bis.b	#BIT4, &P1REN			; enable pull-up/down
			bis.b	#BIT4, &P1OUT			; P1.4 -> pull-up

			bic.b	#BIT4, &P1IFG			; inicijalno brisanje flega prekida
			bis.b	#BIT4, &P1IE			; dozvola prekida na P1.4
			bis.b	#BIT4, &P1IES			; opadajuca ivica generise prekid

			; LED3
			bis.b	#BIT4, &P2DIR			; P2.4 -> output
			bic.b	#BIT4, &P2OUT			; P2.4 -> pull-down

			; Switch 4
			bic.b	#BIT5, &P1DIR			; P1.5 -> input
			bis.b	#BIT5, &P1REN			; enable pull-up/down
			bis.b	#BIT5, &P1OUT			; P1.5 -> pull-up

			bic.b	#BIT5, &P1IFG			; inicijalno brisanje flega prekida
			bis.b	#BIT5, &P1IE			; dozvola prekida na P1.4
			bis.b	#BIT5, &P1IES			; opadajuca ivica generise prekid

			; LED4
			bis.b	#BIT5, &P2DIR			; P2.5 -> output
			bic.b	#BIT5, &P2OUT			; P2.5 -> pull-down
			nop
			bis.b	#GIE, SR				; dozvola svih prekida
			nop
;-----------------------------------------------------------------------------------
; Main loop here
;-----------------------------------------------------------------------------------
			mov 	#0x00, R10				; pocetna vrednost R10 na 0
			call 	#WriteLed

opet		call	#LED_on_off
			jmp 	opet

			.text
PORT1_ISR	bit.b	#0x30, &P1IFG			; provera porekla zahteva za prekid
			jz		exit

			mov		#0xfff, R9				; cekanje da se stanje smiri nakon
wait		dec		R9                      ; pritiska tastera
			jnz		wait

			bit.b	#BIT4, &P1IN			; da li je inc taster jos uvek pritisnut
			jz		T4on
			bit.b	#BIT5, &P1IN			; da li je dec taster jos uvek pritisnut
			jz		T5on

Toff		jmp		exit
T4on		xor.b	#BIT4, &P2OUT			; menja stanje LED1
			inc		R10						; inkrementira vrednost u R10
			;bic.b	#0xf0, R10
			and.b	#0x0f, R10				; propusta samo niza 4 bita
			jmp		Write

T5on		xor.b	#BIT5, &P2OUT			; menja stanje LED2
			dec		R10						; dekrementira vrednost u R10
			and.b	#0x0f, R10				; propusta samo niza 4 bita

Write		call 	#WriteLed				; poziva funkciju WriteLed
exit		bic.b	#BIT4, &P1IFG			; brisanje flega koji oznacava na
			bic.b	#BIT5, &P1IFG			; kom se pinu dogodio prekid
			reti

;-----------------------------------------------------------------------------------
; Stack Pointer definition
;-----------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack
            
;-----------------------------------------------------------------------------------
; Interrupt Vectors
;-----------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET
            
            .sect 	".int47"				; Interrupt na pinu 1
            .short	PORT1_ISR

