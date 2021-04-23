;---------------------------------------------------------------------------------------------
;
; @file LED_on_off.asm
; @brief Implementation of function that turns on LED1 as long as S1 button is pressed and
; LED2 as long as S2 button is pressed.
;
; 4.2 and 4.3
;
; @date 23.04.2021
; @author Andrea Ciric (andreaciric23@gmail.com)
;
; @version [1.0 - 04/2021] Initial version
;
;---------------------------------------------------------------------------------------------

			.cdecls	C,LIST,"msp430.h"

			.def 	LED_on_off

;---------------------------------------------------------------------------------------------
			.text
LED_on_off:
			; 4.2
setup:		bic.b	#BIT1, &P2DIR			; P2.1 (S1) -> input
			bis.b	#BIT1, &P2REN			; enable pull-up/down
			bis.b	#BIT1, &P2OUT			; P2.1 -> pull-up

			bis.b	#BIT0, &P1DIR			; P1.0 (LED1) -> output
			bic.b	#BIT0, &P1OUT			; P1.0 -> pull-down

			; 4.3
			bic.b	#BIT1, &P1DIR			; P1.1 (S2) -> input
			bis.b	#BIT1, &P1REN			; enable pull-up/down
			bis.b	#BIT1, &P1OUT			; P1.1 -> pull-up

			bis.b	#BIT7, &P4DIR			; P4.7 (LED2) -> output
			bic.b	#BIT7, &P4OUT			; P4.7 -> pull-down

;---------------------------------------------------------------------------------------------

loop		bit.b	#BIT1, &P2IN			; testira bit P2.1 (taster S1)
			jz		ledon1
			bic.b	#BIT0, &P1OUT			; iskljucuje LED1

			bit.b	#BIT1, &P1IN			; testira bit P1.1 (taster S2)
			jz 		ledon2
			bic.b	#BIT7, &P4OUT			; iskljucuje LED2
			jmp		loop

ledon1		bis.b	#BIT0, &P1OUT			; ukljucuje LED1

			; ponovo ispitujemo P1.1 da bismo pokrili i slucaj
			; da se S2 pritisne dok je pritisnut S1

			bit.b	#BIT1, &P1IN			; testira bit P1.1 (taster S2)
			jz 		ledon2
			bic.b	#BIT7, &P4OUT			; iskljucuje LED2
			jmp		loop

ledon2		bis.b	#BIT7, &P4OUT			; ukljucuje LED2
			jmp		loop
			nop


			ret

			.end
