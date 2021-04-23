#include <msp430.h> 


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	// GPIO initialization
	// LED1
	P2OUT &= ~BIT4;             // clear P2.4
	P2DIR |= BIT4;              // configure P2.4 as out

	// Switch S1
	P1DIR &= ~BIT4;             // by default the pin is in
	                            // configure is just for demo
	P1REN |= BIT4;              // enable pull up/down
	P1OUT |= BIT4;              // configure pull up

	// main loop
	while(1)
	{
	    // compare values of LED state and switch state
	    // NOTE: switch is '0' when pressed!
	    if ((P2OUT & BIT4) ^ (~P1IN & BIT4))
        {
            P2OUT ^= BIT4;
        }
	}
/*
    while (1)
    {
        if ((P1IN & BIT4) != 0)
        {
            // button is not pressed, turn off LED
            P2OUT &= ~BIT4;
        }
        else
        {
            // button is pressed, turn on LED
            P2OUT |= BIT4;
        }
    }
*/
	//return 0;
}
