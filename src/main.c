#include <msp430.h> 

#include <assert.h>



void initAllGpioToOutputsLow()
{
    P1DIR = 0xFB; P2DIR = 0xFF; P3DIR = 0xFF;
    P1REN = 0xFF; P2REN = 0xFF; P3REN = 0xFF;
    P1OUT = 0x00; P2OUT = 0x00; P3OUT = 0x00;

    // configuration not effective until also call unlockLPM5()
}


/*
 * _low_power_mode_4() locks this bit, to enter LPM4.5
 * It can only be cleared by power cycle or in software (as done here.)
 */
void unlockLPM5() {
    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
}






/*
 * Button on MSP-EXP430FR2433Launchpad
 * P2.3
 *
 * There is no separate interrupt clearing method:  see below, no ISR is called.
 */
void configureButtonGPIOForInterrupt() {
    P2DIR &= ~(BIT3);                   // input direction
    P2OUT |= BIT3;                      // pull is up
    P2REN |= BIT3;                      // pull-up enable
    P2IES |= BIT3;                      // Hi to Low edge

    // Button configured but interrupt not enabled.
}

void enableButtonInterrupt() {
    P2IFG = 0;                          // Clear all P1 interrupt flags
    P2IE |= BIT3;                       // P1.3 interrupt enabled
}





void blinkRedLed() {
    P1OUT |= BIT0;
    __delay_cycles(200000); // 0.2 seconds at 1Mhz clock
    P1OUT &= ~(BIT0);
}

void blinkGreenLed() {
    P1OUT |= BIT1;
    __delay_cycles(200000); // 0.2 seconds at 1Mhz clock
    P1OUT &= ~(BIT1);
}

//  P1OUT ^= BIT0;                      // P1.0 = toggle




int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop WDT

    // RTC is not counting
    assert(RTCCTL == 0);

    /*
     * A reset occurred and the config registers of GPIO define them as inputs.
     * !!! But the locked configuration is different.
     *
     * Two basic cases:
     * - wake from LPM4.5 reset
     *     LPM5 is locked (actual GPIO state locked, but GPIO config registers say inputs)
     *     some interrupt flag is set
     * - cold reset (power on)
     *     LPM5 is not locked
     */

    initAllGpioToOutputsLow();
    configureButtonGPIOForInterrupt();
    /*
     * GPIO config is the sleeping config: LED and button.  But config is not effective yet.
     */

    /*
     * In one case (cold reset) LPM5 is not already locked.  But it is convenient to unlock it anyway.
     */
    unlockLPM5();
    /*
     * An interrupt was enabled before we slept LPM5.
     * If this is a wake from LPM5, the signal from the button woke us,
     * but since we did not enable the interrupt after we woke,
     * no ISR will be called.
     * If we had enabled the interrupt after waking,
     * the ISR would be called now (before the next instruction.)
     */

    // Determine whether we are coming out of an LPMx.5 or a regular RESET.
    // Side effect of reading SYSRSTIV is to clear the highest priority reason (but not all of them?)
    if (SYSRSTIV == SYSRSTIV_LPM5WU)        // MSP430 just woke up from LPMx.5
    {
        // Wake from LPM5

        // Example
        blinkRedLed();
    }
    else
    {
        // Device powered up from a cold start.
        blinkGreenLed();
    }

    /*
     * Enable a source of a wake interrupt.
     */
    enableButtonInterrupt();


    /*
     * This is what makes the mode x.5:  unpower the core
     */
    PMMCTL0_H = PMMPW_H;                // Open PMM Registers for write
    PMMCTL0_L |= PMMREGOFF;             // Set PMMREGOFF
    PMMCTL0_H = 0;                      // Lock PMM Registers

    /*
     * Enter LPM4. (more precisely, 4.5 since we unpowered core regulator above.)
     *  Note that this operation does not return.
     *  Will wake from LPM4.5 via a RESET event, resulting in a re-start of the code.
     *
     *  !!! This must closely follow clearing PMMREGOFF (with a limit on instruction count in between),
     *  since the core will soon stop.
     *
     *  This is equivalent to __bis_SR_register(LPM4_bits | GIE);
     */
    _low_power_mode_4();

}

