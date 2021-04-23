/**
 * @file main.c
 * @brief Demo Button Toggle ISR
 *
 * In this example one LED is toggled every time one button is pressed.
 * Button press is detected using interrupt.
 *
 * @date 2021
 * @author Strahinja Jankovic (jankovics@etf.bg.ac.rs)
 * @author Marija Bezulj (meja@etf.bg.ac.rs)
 *
 * @version [2.0 - 04/2021] Hardware change
 * @version [1.1 - 03/2019] Code refactoring
 * @version [1.0 - 03/2017] Initial version
 */

#include <msp430.h>

/**
 * @brief Main function
 *
 * Main function only initializes ports and enables global interrupts
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // GPIO initialization
    // LED1
    P2OUT &= ~BIT4;             // clear P2.4
    P2DIR |= BIT4;              // configure P2.4 as out

    // Switch S1
    P1DIR &= ~BIT5;             // configure P1.5 as in
    P1REN |= BIT5;              // enable pull up/down
    P1OUT |= BIT5;              // configure pull up

    P1IES |= BIT5;              // falling edge interrupt
    P1IFG &= ~BIT5;             // clear P1.5 flag
    P1IE  |= BIT5;              // interrupt on P1.5 enabled

    __enable_interrupt();       // setuje General Interrupt
                                // Enable bit u SR

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
            P2OUT ^= BIT4;          // toggle P2.4
        }
        P1IFG &= ~BIT5;             // clear P1.5 flag
    }

    return;
}
