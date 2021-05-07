/**
 * @file main.c
 * @brief Demo Timer multiplex
 *
 * In this example number 12 is displayed on multiplexed 7 segment LED display
 *
 * @date 2021
 * @author  Marija Bezulj [meja@etf.bg.ac.rs]
 * @modifiedBy Andrea Ciric
 *
 */

#include <msp430.h> 
#include <stdint.h>
#include "writeLed.h"


/**
 * @brief Timer period
 *
 * Timer is clocked by ACLK (32768Hz)
 * It takes 32768 cycles to count to 1s.
 * If we need a period of X ms, then number of cycles
 * that is written to the CCR0 register is
 * 32768/1000 * X
 */
#define TIMER_PERIOD        (163)  /* ~5ms (4.97ms)  */

/**
 * @brief Number to be displayed
 */
#define NUMBER          (23)

/** macro to convert ASCII code to digit */
#define ASCII2DIGIT(x)      (x - '0')
/** macro to convert digit to ASCII code */
#define DIGIT2ASCII(x)      (x + '0')

/** variable where received character is placed */
volatile uint8_t data = 0;

/**
 * @brief digits used for display [disp2 disp1]
 */
volatile uint8_t disp2, disp1;

/**
 * @brief Function that extracts digits from number
 * @author Strahinja Jankovic
 * @modifiedBy Andrea Ciric
 */
void display(const uint16_t number)
{
    uint16_t nr = number;
    uint8_t tmp;

    //double dabble
    uint8_t data[2] = {0};
    int8_t count;

    for (count = 7; count >= 0; count--)
    {
        uint8_t next = (nr & BIT7) ? 1 : 0;
        nr <<= 1;
        for (tmp = 0; tmp < 2; tmp++)
        {
            uint8_t nibble = data[tmp];
            nibble += (nibble >= 5) ? 3 : 0;
            nibble <<= 1;
            nibble |= next;
            next = (nibble & BIT4) ? 1 : 0;
            data[tmp] = nibble & 0xf;
        }
    }

    disp2 = data[1];
    disp1 = data[0];

}

/**
 * @brief Main function
 * Initialize the 7seg display and timer in compare mode.
 * ISR will multiplex the display
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    // sevenseg 1
    P7DIR |= BIT0;              // set P7.0 as out (SEL1)
    P7OUT |= BIT0;              // disable display 1
    // sevenseg 2
    P6DIR |= BIT4;              // set P6.4 as out (SEL2)
    P6OUT |= BIT4;              // disable display 2

    // a,b,c,d,e,f,g
    P2DIR |= 0x48;              // configure P2.3 and P2.6 as out
    P3DIR |= BIT7;              // configure P3.7 as out
    P4DIR |= 0x09;              // configure P4.0 and P4.3 as out
    P8DIR |= 0x06;              // configure P8.1 and P8.2 as out

    // init TA1 as compare in up mode
    TA1CCR0 = TIMER_PERIOD;         // set timer period in CCR0 register
    TA1CCTL0 = CCIE;                // enable interrupt for TA1CCR0
    TA1CTL = TASSEL__ACLK | MC__UP; //clock select and up mode

    // create BCD digits
    display(NUMBER);

    // initialize USCI UART A1
    P4SEL |= BIT4 | BIT5;       // select P4.4 and P4.5 for USCI

    UCA1CTL1 |= UCSWRST;        // put USCI in reset

    UCA1CTL0 = 0;               // no parity, 8bit, 1 stop bit
    UCA1CTL1 |= UCSSEL__SMCLK;   // use MCLK ~1MHz
    UCA1BR0 = 52;               // 52 < 2^8 = 256
    UCA1BR1 = 0;
    UCA1MCTL = 0;               // BRS = 0 & BRF = 0

    UCA1CTL1 &= ~UCSWRST;       // release reset

    UCA1IE |= UCRXIE;           // enable RX interrupt

    __enable_interrupt();       // GIE

    while(1);
}

/**
 * @brief USCIA1 ISR
 *
 * When data is received using UART, save it and display on 7seg display
 */
void __attribute__ ((interrupt(USCI_A1_VECTOR))) UARTISR (void)
{
    if (UCA1IV == USCI_UCRXIFG)
    {
        data = ASCII2DIGIT(UCA1RXBUF);  // save data
        //display(UCA1RXBUF);     // write to 7seg
        WriteLed(data);
    }
}


/**
 * @brief TA0CCR0 ISR
 *
 * Multiplex the 7seg display. Each ISR activates one digit.
 */
void __attribute__ ((interrupt(TIMER1_A0_VECTOR))) CCR0ISR (void)
{
    static uint8_t current_digit = 0;

    /* algorithm:
     * - turn off previous display (SEL signal)
     * - set a..g for current display
     * - activate current display
     */
    if (current_digit == 1)
    {
        P6OUT |= BIT4;          // turn off SEL2
        WriteLed(disp2);        // define seg a..g
        P7OUT &= ~BIT0;         // turn on SEL1
    }
    else if (current_digit == 0)
    {
        P7OUT |= BIT0;
        WriteLed(disp1);
        P6OUT &= ~BIT4;
    }
    current_digit = (current_digit + 1) & 0x01;

    return;
}

