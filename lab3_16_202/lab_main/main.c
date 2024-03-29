/**
 * @file main.c
 * @brief ADC, PWM and UART
 *
 * Timer B0 periodically (16Hz) triggers the conversion
 * on channel A0 of ADC12, which is connected to a potentiometer.
 * 8 greater bits from 12 bits of the conversion result are sent
 * using UART to PC everytime PC sends 's' (0x73) and at the same
 * time defining the duty cycle of PWM on TA0CCR2 OUT. On every 's'
 * PWM duty cycle gets inverted.
 *
 * @date 15.05.2021.
 * @author  Andrea Ciric (andreaciric23@gmail.com)
 *
 * @version [1.0 - 05/2021] Initial version for MSP430F5529
 *
 */
#include <msp430.h> 
#include <stdint.h>

#define BR9600      (3)         //ACLK
#define BRS9600      (UCBRS_3)  //ACLK

/**
 * @brief Timer period for ADC12 conversion triggering
 *
 * Timer is clocked by ACLK (32768Hz).
 * No divider is set.
 * We want 16Hz frequency, so use 2048 for CCR1
 */
#define CONV_PERIOD        (2048)  /* ~62.5 ms */

/*
 * Timer is clocked by ACLK (32768Hz)
 * We want fs = 1Hz => period 1s, so use 32768 for CCR0
 */
#define PWM_PERIOD      (32768)  /* 1s */

/**
 * @brief Timer period
 *
 * Timer is clocked by ACLK (32768Hz).
 * We want ~5ms period, so use 163 for CCR0
 */
#define TIMER_PERIOD        (163)  /* ~5ms (4.97ms) */


volatile unsigned int ad_result = 0;        // variable where conversion result is placed
volatile uint16_t dutyclc = 0;              // variable where duty cycle is placed
volatile uint8_t temp = 0;                  // variable where data received in Rx is saved

/**
 * @brief Main function
 *
 * Peripheral initialization. USCI_A1 is used in UART mode
 * for transfer of ADC12 conversion result.
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    // Initialize UART
    P4SEL |= BIT4 | BIT5;           // enable P4.4 and P4.5 for UART

    UCA1CTL1 |= UCSWRST;            // set software reset

    UCA1CTL0 = 0;                   // no parity, 8bit, 1 stop bit
    UCA1CTL1 |= UCSSEL__ACLK;       // use ACLK = 32 768 Hz
    UCA1BRW = BR9600;               // BR = 3
    UCA1MCTL |= BRS9600 + UCBRF_0;   // BRS = 3 for 9600 bps

    UCA1CTL1 &= ~UCSWRST;           // release software reset

    UCA1IE |= UCRXIE | UCTXIE;      // enable RX and TX interrupt


    /* REF module */
    REFCTL0 &= ~REFMSTR;        // ref system controlled by legacy control bits inside ADC12_A

    /* ADC12_A channel A0 init */
    P6SEL |= BIT0;              // set pin P6.0 as alternate function - A0 analog
                                // this is pot2 potentiometer
    ADC12CTL0 &= ~ADC12ENC;     // disable ADC before configuring
    /* 32 cycles for sampling, ref voltage is set by ADC12_A, ADC ON */
    ADC12CTL0 |= ADC12SHT0_3 + ADC12REF2_5V + ADC12ON;
    ADC12CTL1 |= ADC12CSTARTADD_0 + ADC12SHS_2 + ADC12SHP + ADC12SSEL_0 + ADC12CONSEQ_2; // start address ADC12MEM0, timer starts conversion
                                                                                         // single channel single conversion, MODCLK clock
                                                                                         // SAMPCON sourced from the sampling timer
    ADC12CTL2 |= ADC12RES_2;    // 12 bit conversion result
    ADC12MCTL0 |= ADC12INCH_0;  // reference AVCC and AVSS, channel A0

    ADC12IE |= ADC12IE0;        // enable interrupt request for the ADC12IFG0 bit
    //ADC12IFG &= ~ADC12IFG0;     // clear ADC12MEM0 interrupt flag

    ADC12CTL0 |= ADC12ENC;      // enable ADC12

    /* initialize Timer B0 for ADC12 trigger */
    // CBOUT CCR0 TB0 -> ADC12SHS2 this is internal, ports and pins not used (P5.6)
    TB0CCR0 = (CONV_PERIOD/2  - 1);// f_OUT = 16 Hz, f_ACLK = 32768 Hz ID=1 => T_OUT = 2048.
                                   // TOGGLE outmod => T_OUT = 2 * T_CCR0 =>
                                   // T_CCR0 = T_OUT/2 = 1024 (TA0CCR0 = T_CCR0 - 1)
    TB0CCTL0 |= OUTMOD_4;    // toggle mode
    TB0CTL |= TBSSEL__ACLK + ID__1 + MC__UP;

    /* init timerA0 with PWM out on LD2 through TA0 CCR2 */

    TA0CCR0 = PWM_PERIOD - 1;       // init pwm period
    // timer is in compare mode with active OUT signa
    TA0CCR2 = dutyclc;          // initial state is no pulse
    TA0CCTL2 = OUTMOD_7;        // outmode is Reset/Set
                                // CCR2 value defines the pulse width
    // init P1.3 pin as alternate function pin
    P1SEL |= BIT3;              // alternate function
    P1DIR |= BIT3;              // P1.3 is TA0.2 pin
    // activate timer
    TA0CTL = TASSEL__ACLK | MC__UP;


    __enable_interrupt();       // GIE

    while(1){
    }
}

/**
 * @brief ADC ISR
 *
 * On ADC12IFG0 save ADC12MEM0
 */
void __attribute__ ((interrupt(ADC12_VECTOR))) ADC12ISR (void)
{
    switch (ADC12IV)
    {
    case ADC12IV_ADC12IFG0:
        // change TA0CCR2 duty cycle on the run, timer stop not needed

        ad_result = ADC12MEM0;
        dutyclc = (ad_result & 0xfff) * (PWM_PERIOD/0xfff);
        TA0CCR2 = dutyclc;
        break;
    default:
        break;
    }
}

/**
 * @brief USCI UART ISR
 */
void __attribute__ ((interrupt(USCI_A1_VECTOR))) UARTISR (void)
{
    switch (UCA1IV)
    {
    case 0:
        break;
    case USCI_UCRXIFG:                      // on Rx interrupt flag do:
        temp = UCA1RXBUF;                   // store Rx value when interrupt happens
        if (temp == 0x73)                   // wait for 's' to be received
        {
            UCA1TXBUF = ad_result >> 4;
            // change TA0CCTL2 output mode on the run, timer stop not needed
            /* A safe method for switching between output modes is to
                use output mode 7 as a transition state => as we are already operating
                with mode 7 it is fine (S/R -> 011b, R/S -> 111b)*/
            TA0CCTL2 ^= OUTMOD_4;           // 011 xor 100 -> 111 xor 100 -> 011...
        }
        break;
    case USCI_UCTXIFG:                      // on Tx interrupt flag do:
        /* UCTXIFG is automatically reset if a character is written to UCAxTXBUF. */
        break;
    }
}
