 /**
 * @file main.c
 * @brief Demo Button 7seg
 *
 * In this example value stored in register R12 is displayed on a sevenseg
 * display every time one button is pressed.
 * Button press is detected using interrupt.
 *
 * @date 2021
 * @author Strahinja Jankovic (jankovics@etf.bg.ac.rs)
 * @author Marija Bezulj (meja@etf.bg.ac.rs)
 *
 * @version [2.0 - 04/2021] Hardware change
 * @version [1.1 - 03/2017] Code refactoring
 * @version [1.0 - 03/2017] Initial version
 */
#include <msp430.h>
#include <stdint.h>
#include "function.h"

volatile uint16_t bcd = 5;

/**
 * @brief Main function
 *
 * Main function only initializes ports and enables global interrupts
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // GPIO initialization
    // Switch S1
    P1DIR &= ~BIT5;             // configure P1.5 as in
    P1REN |= BIT5;              // enable pull up/down
    P1OUT |= BIT5;              // configure pull up
    P1IES |= BIT5;              // falling edge interrupt
    P1IFG &= ~BIT5;             // clear P1.5 flag
    P1IE  |= BIT5;              // interrupt on P1.5 enabled

    // sevenseg 1
    P7DIR |= BIT0;              //set P7.0 as out (SEL1)
    P7OUT &= ~BIT0;              //enable diplay 1

    // a,b,c,d,e,f,g
    P2DIR |= 0x48;              // configure P2.3 and P2.6 as out
    P3DIR |= BIT7;              // configure P3.7 as out
    P4DIR |= 0x09;              // configure P4.0 and P4.3 as out
    P8DIR |= 0x06;              // configure P8.1 and P8.2 as out

    __enable_interrupt();

    //main loop
    while(1);
}

/**
 * @brief PORT2 Interrupt service routine
 *
 * ISR debounces P1.5 and toggles LED if falling edge is detected
 */
void __attribute__ ((interrupt(PORT1_VECTOR))) P1ISR (void)
{
    __delay_cycles(1000);           // ~1ms

    if ((P1IFG & BIT5) != 0)        // check if P1.5 flag is set
    {
        if ((P1IN & BIT5) == 0)     // check if P1.5 is still pressed
        {
            WriteLed(bcd);          // call WriteLed
                                    // parameter is passed through R12
        }
        P1IFG &= ~BIT5;             // clear P1.5 flag
    }

    return;
}
