;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file


;-------------------------------------------------------------------------------
			.ref	WriteLed
			.ref	disp2
			.ref	disp1
			.ref	current_digit


;-------------------------------------------------------------------------------
			.text

TIMERA1_ISR	ADD		&TA1IV,	PC					; Add offset to Jump table

			CMP.B	#1,	&current_digit
			JEQ		DIGIT1

			CMP.B	#0, &current_digit
			JEQ		DIGIT0

DIGIT1		BIS.B	#BIT4,	&P6OUT
			MOV.B	&disp2,	R12
			CALL	#WriteLed
			BIC.B	#BIT0,	&P7OUT
			JMP 	exit

DIGIT0		BIS.B	#BIT0,	&P7OUT
			MOV.B	&disp1,	R12
			CALL	#WriteLed
			BIC.B	#BIT4,	&P6OUT


exit		MOV.W	#1, R15
			XOR.B	&current_digit,	R15
			AND.B	#1,	R15
			MOV.B	R15, &current_digit
			;POPM.A	#5,	R15

			reti


;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".int49"                ; MSP430 TIMER1_A0_VECTOR
            .short  TIMERA1_ISR
