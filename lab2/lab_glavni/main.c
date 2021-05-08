/**
 * @file main.c
 * @brief Exchanging data with computer using UART communication.
 * Data received in a package (in the form of 's'XY't', X and Y being digits 0-9) is shown on the 2 7-seg displays.
 * Received data is "echoed back" to Tx.
 *
 *
 * @date 08.05.2021.
 * @author  Andrea Ciric (andreaciric23@gmail.com)
 *
 * @version [1.0 - 05/2021] Initial version for MSP430F5529
 *
 */

#include <msp430.h> 
#include <stdint.h>
#include "writeLed.h"


/**
 * @brief Timer period for 2 7-seg displays mux
 *
 * Timer is clocked by ACLK (32768Hz)
 * It takes 32768 cycles to rx_cnt to 1s.
 * If we need a period of X ms, then number of cycles
 * that is written to the CCR0 register is
 * 32768/1000 * X
 */
#define TIMER_PERIOD        (163)  /* ~5ms (4.97ms)  */

#define ASCII2DIGIT(x)      (x - '0')   // macro to convert ASCII code to digit
#define DIGIT2ASCII(x)      (x + '0')   // macro to convert digit to ASCII code

//#define NUMBER          (23)          // Number to be displayed in 5.1

//volatile uint8_t data = 0;            // variable where received character in 5.3 is placed

volatile uint8_t disp2, disp1;          // digits used for display (order: [disp2 disp1])
volatile uint8_t temp = 0;              // variable where data received in Rx is saved

volatile uint8_t rx_cnt = 0;            // variable for counting data received
volatile uint8_t tx_cnt = 0;            // variable for counting data sent

volatile uint8_t digits[2];             // variable where two digits received from rx are placed
volatile uint8_t PCK_ARRIVED = 0;       // flag that says if packet has arrived


/**
 * @brief Function that extracts digits from number
 */
void display(const uint16_t number)
{
    uint16_t nr = number;
    uint8_t tmp;

    //double dabble
    uint8_t data[2] = {0};
    int8_t rx_cnt;

    for (rx_cnt = 7; rx_cnt >= 0; rx_cnt--)
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
    //display(NUMBER);

    // initialize USCI UART A1
    P4SEL |= BIT4 | BIT5;           // select P4.4 and P4.5 for USCI

    UCA1CTL1 |= UCSWRST;            // put USCI in reset

    UCA1CTL0 = 0;                   // no parity, 8bit, 1 stop bit
    UCA1CTL1 |= UCSSEL__SMCLK;      // use SMCLK ~1048576Hz
    UCA1BR0 = 54;                   // 52 < 2^8 = 256
    UCA1BR1 = 0;
    UCA1MCTL |= UCBRS_5 + UCBRF_0;  // BRS = 5 & BRF = 0

    UCA1CTL1 &= ~UCSWRST;           // release reset

    UCA1IE |= UCRXIE + UCTXIE;      // enable RX interrupt

    __enable_interrupt();           // GIE

    while(1)
    {
        if (PCK_ARRIVED == 1)       // if pck arrived send first char to Tx
        {                           // remaining chars are sent through ISR
            PCK_ARRIVED = 0;
            tx_cnt = 1;
            UCA1TXBUF = 's';
        }
    }
}

/**
 * @brief USCIA1 ISR
 *
 * When data is received using UART in the form of 's'XY't', save it,
 * display it on 7seg display and echo it back to Tx.
 */
void __attribute__ ((interrupt(USCI_A1_VECTOR))) UARTISR (void)
{
    switch (UCA1IV)
    {
    case 0:
        break;
    case USCI_UCRXIFG:                      // on Rx interrupt flag do:
        //5.3
        //data = ASCII2DIGIT(UCA1RXBUF);    // save data
        //display(UCA1RXBUF);               // write to 7seg

        temp = UCA1RXBUF;
        if ((temp == 0x73) && (rx_cnt == 0))        // wait for 's' to be received
            rx_cnt++;
        else if ((rx_cnt >= 1) && (rx_cnt < 3))     // save next two chars
        {
            digits[rx_cnt - 1] = temp;
            rx_cnt++;
        }
        else if (rx_cnt == 3)
        {
            if (temp == 0x74)                       // check if 4th char is 't'
            {
                disp2 = ASCII2DIGIT(digits[0]);     // if yes display it
                disp1 = ASCII2DIGIT(digits[1]);
                PCK_ARRIVED = 1;                    // notify that pck arrived
            }
            rx_cnt = 0;                             // else reset the counter
        }
        break;
    case USCI_UCTXIFG:                      // on Tx interrupt flag do:
        if (tx_cnt == 1)                    // send 2nd char (first one was sent in the main function)
        {
            UCA1TXBUF = digits[0];
            tx_cnt++;
        }
        else if (tx_cnt == 2)               // send 3rd char
        {
            UCA1TXBUF = digits[1];
            tx_cnt++;
        }
        else if (tx_cnt == 3)               // send 4th char
        {
            UCA1TXBUF = 't';
            tx_cnt = 0;                     // reset counter
        }
        break;
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
