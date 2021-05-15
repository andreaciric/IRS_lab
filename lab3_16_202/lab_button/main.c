/**
 * @file main.c
 * @brief Demo ADC12 single conversion on button press
 *
 * Press of a button initiates conversion on channel A0
 * of ADC12, which is connected to a potentiometer.
 * Result of the conversion is sent through USCI to PC.
 *
 * @date 2021
 * @author Marija Bezulj (meja@etf.bg.ac.rs)
 *
 * @version [1.0 @ 05/2021] Initial version
 */
#include <msp430.h> 
#include <stdint.h>

#define BR9600      (109)

#define BRS9600      (UCBRS_2)

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

/** variable where conversion result is placed */
volatile unsigned int ad_result = 0;

/** variable where duty cycle is placed */
volatile uint16_t dutyclc = 0;

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
    UCA1CTL1 |= UCSSEL__SMCLK;      // use SMCLK = 1 048 576 Hz
    UCA1BRW = BR9600;               // BR = 109
    UCA1MCTL |=BRS9600 + UCBRF_0;   // BRS = 2 for 9600 bps

    UCA1CTL1 &= ~UCSWRST;           // release software reset

    // configure button S1
    P1REN |= BIT4;              // enable pull up/down
    P1OUT |= BIT4;              // set pull up
    P1DIR &= ~BIT4;             // configure P1.4 as in
    P1IES |= BIT4;              // interrupt on falling edge
    P1IFG &= ~BIT4;             // clear flag
    P1IE  |= BIT4;              // enable interrupt

    /* initialize Timer A1 */
    TA1CCR0 = TIMER_PERIOD;     // debounce period
    TA1CCTL0 |= CCIE;           // enable CCR0 interrupt
    TA1CTL |= TASSEL__ACLK;

    /* REF module */
    REFCTL0 &= ~REFMSTR;        // ref system controlled by legacy control bits inside ADC12_A

    /* ADC12_A channel A0 init */
    P6SEL |= BIT0;              // set pin P6.0 as alternate function - A0 analog
                                // this is pot2 potentiometer
    ADC12CTL0 &= ~ADC12ENC;     // disable ADC before configuring
    /* 32 cycles for sampling, ref voltage is set by ADC12_A, ADC ON */
    ADC12CTL0 |= ADC12SHT0_3 + ADC12REF2_5V + ADC12ON;
    ADC12CTL1 |= ADC12CSTARTADD_0 + ADC12SHP + ADC12SSEL_0 + ADC12CONSEQ_0;  // start address ADC12MEM0, SC starts conversion
                                                                             // single channel single conversion, MODCLK clock
                                                                             // SAMPCON sourced from the sampling timer ?
    ADC12CTL2 |= ADC12RES_2;    // 12 bit conversion result
    ADC12MCTL0 |= ADC12INCH_0;  // reference AVCC and AVSS, channel A0

    ADC12IE |= ADC12IE0;        // enable interrupt request for the ADC12IFG0 bit
    //ADC12IFG &= ~ADC12IFG0;     // clear ADC12MEM0 interrupt flag

    ADC12CTL0 |= ADC12ENC;      // enable ADC12

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


    __enable_interrupt();           // GIE

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
        UCA1TXBUF = ad_result;

        //data = ADC12MEM0 & 0xff;
        //ndrf = 1;
        dutyclc = (ad_result & 0xfff) * (PWM_PERIOD/0xfff);
        TA0CCR2 = dutyclc;
        break;
    default:
        break;
    }
}


/**
 * @brief PORT1 ISR
 *
 * On button press send data using UART
 */
void __attribute__ ((interrupt(PORT1_VECTOR))) P1ISR (void)
{
    if ((P1IFG & BIT4) != 0)        // check if P1.4 flag is set
    {
        /* start timer */
        TA1CTL |= MC__UP;

        P1IFG &= ~BIT4;             // clear P1.4 flag
        P1IE &= ~BIT4;              // disable P1.4 interrupt
    }
}

/**
 * @brief TIMERA0 Interrupt service routine
 *
 * ISR debounces P1.4 and starts ADC12 single conversion
 */
void __attribute__ ((interrupt(TIMER1_A0_VECTOR))) CCR0ISR (void)
{
    if ((P1IN & BIT4) == 0) // check if button is still pressed
    {
        ADC12CTL0 |= ADC12SC;  // start ADC12 conversion
    }
    TA1CTL &= ~(MC0 | MC1); // stop and clear timer
    TA1CTL |= TACLR;
    P1IFG &= ~BIT4;         // clear P1.4 flag
    P1IE |= BIT4;           // enable P1.4 interrupt
    return;
}
