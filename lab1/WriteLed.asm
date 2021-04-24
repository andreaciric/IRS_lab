;---------------------------------------------------------------------------------------------
;
; @file WriteLed.asm
; @brief Implementation of function used to write (0-F) to 7seg display
;
; 4.1
;
; @date 23.04.2021
; @author Andrea Ciric (andreaciric23@gmail.com)
;
; @version [1.0 - 04/2021] Initial version
;
;---------------------------------------------------------------------------------------------

			.cdecls	C,LIST,"msp430.h"

			.def WriteLed

;---------------------------------------------------------------------------------------------
; Digits const table
;
			.sect	.const

segtab_p2	.byte	0x48	; 0				; digit 0-9 table
			.byte	0x40	; 1
			.byte	0x08	; 2
			.byte	0x40	; 3
			.byte	0x40	; 4
			.byte	0x40	; 5
			.byte	0x48	; 6
			.byte	0x40	; 7
			.byte	0x48	; 8
			.byte	0x40	; 9
			.byte	0x48	; A
			.byte	0x48	; B
			.byte	0x08	; C
			.byte	0x48	; D
			.byte	0x08	; E
			.byte	0x08	; F

segtab_p3	.byte	0x80	; 0				; digit 0-9 table
			.byte	0x00	; 1
			.byte	0x80	; 2
			.byte	0x80	; 3
			.byte	0x00	; 4
			.byte	0x80	; 5
			.byte	0x80	; 6
			.byte	0x80	; 7
			.byte	0x80	; 8
			.byte	0x80	; 9
			.byte	0x80	; A
			.byte	0x00	; B
			.byte	0x80	; C
			.byte	0x00	; D
			.byte	0x80	; E
			.byte	0x80	; F

segtab_p4	.byte	0x09	; 0				; digit 0-9 table
			.byte	0x08	; 1
			.byte	0x08	; 2
			.byte	0x08	; 3
			.byte	0x09	; 4
			.byte	0x01	; 5
			.byte	0x01	; 6
			.byte	0x08	; 7
			.byte	0x09	; 8
			.byte	0x09	; 9
			.byte	0x09	; A
			.byte	0x01	; B
			.byte	0x01	; C
			.byte	0x08	; D
			.byte	0x01	; E
			.byte	0x01	; F

segtab_p8	.byte	0x02	; 0				; digit 0-9 table
			.byte	0x00	; 1
			.byte	0x06	; 2
			.byte	0x06	; 3
			.byte	0x04	; 4
			.byte	0x06	; 5
			.byte	0x06	; 6
			.byte	0x00	; 7
			.byte	0x06	; 8
			.byte	0x06	; 9
			.byte	0x04	; A
			.byte	0x06	; B
			.byte	0x02	; C
			.byte	0x06	; D
			.byte	0x06	; E
			.byte	0x04	; F

;---------------------------------------------------------------------------------------------
; Prikaz cifre na 7seg displeju

			.text
			;mov.w	#0x03, R10				; vrednost koja se ispisuje na displej
WriteLed	bis.b	#0x48, &P2OUT			; deaktivirnje segmenata c i e na portu 2
			bic.b	segtab_p2(R10), &P2OUT	; indeksiranje tabele segmenata i ispis na displej

			bis.b	#BIT7, &P3OUT			; deaktivirnje segmenata a na portu 3
			bic.b	segtab_p3(R10), &P3OUT	; indeksiranje tabele segmenata i ispis na displej

			bis.b	#0x09, &P4OUT			; deaktivirnje segmenata b i f na portu 4
			bic.b	segtab_p4(R10), &P4OUT	; indeksiranje tabele segmenata i ispis na displej

			bis.b	#0x06, &P8OUT			; deaktivirnje segmenata d i g na portu 8
			bic.b	segtab_p8(R10), &P8OUT	; indeksiranje tabele segmenata i ispis na displej

			;jmp 	lab						; skok na labelu za potrebe debugovanja/simulacije
			;nop

			ret

			.end
