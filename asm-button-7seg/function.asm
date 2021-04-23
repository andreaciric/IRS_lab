; Function that returns 7 segment display from certain digit

			.cdecls C,LIST,"msp430.h"       ; Include device header file

			.def	WriteLed

			.sect	.const

segtab2		.byte	0x48					; digit 0-9 table
			.byte	0x40
			.byte	0x08
			.byte	0x40
			.byte	0x40
			.byte	0x40
			.byte	0x48
			.byte	0x40
			.byte	0x48
			.byte	0x40

segtab3		.byte	0x80					; digit 0-9 table
			.byte	0x00
			.byte	0x80
			.byte	0x80
			.byte	0x80
			.byte	0x80
			.byte	0x80
			.byte	0x80
			.byte	0x80
			.byte	0x80

segtab4		.byte	0x09					; digit 0-9 table
			.byte	0x08
			.byte	0x08
			.byte	0x08
			.byte	0x09
			.byte	0x01
			.byte	0x01
			.byte	0x08
			.byte	0x09
			.byte	0x09

segtab8		.byte	0x02					; digit 0-9 table
			.byte	0x00
			.byte	0x06
			.byte	0x06
			.byte	0x04
			.byte	0x06
			.byte	0x06
			.byte	0x00
			.byte	0x06
			.byte	0x06

			.text
WriteLed	bis.b 	#0x48, &P2OUT
			bic.b	segtab2(R12), &P2OUT

			bis.b 	#0x80, &P3OUT
			bic.b	segtab3(R12), &P3OUT

			bis.b 	#0x09, &P4OUT
			bic.b	segtab4(R12), &P4OUT

			bis.b 	#0x06, &P8OUT
			bic.b	segtab8(R12), &P8OUT

			ret

			.end
