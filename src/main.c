#include <msp430.h> 

#include <assert.h>

#include <stdbool.h>



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
#ifdef __MSP430FR2433__
void configureButtonGPIOForInterrupt() {

    P2DIR &= ~(BIT3);                   // input direction
    P2OUT |= BIT3;                      // pull is up
    P2REN |= BIT3;                      // pull-up enable
    P2IES |= BIT3;                      // Hi to Low edge

    // Button configured but interrupt not enabled.
}

void enableButtonInterrupt() {
    P2IFG = 0;                          // Clear all P2 interrupt flags
    P2IE |= BIT3;                       // P2.3 interrupt enabled
}
#else
void configureButtonGPIOForInterrupt() {

    P1DIR &= ~(BIT1);                   // input direction
    P1OUT |= BIT1;                      // pull is up
    P1REN |= BIT1;                      // pull-up enable
    P1IES |= BIT1;                      // Hi to Low edge

    // Button configured but interrupt not enabled.
}

void enableButtonInterrupt() {
    P1IFG = 0;                          // Clear all P1 interrupt flags
    P1IE |= BIT1;                       // interrupt enabled
}
#endif





void blinkRedLed() {
    P1OUT |= BIT0;
    __delay_cycles(200000); // 0.2 seconds at 1Mhz clock
    P1OUT &= ~(BIT0);
}

void blinkGreenLed() {
#ifdef __MSP430FR2433__
    P1OUT |= BIT1;
    __delay_cycles(200000); // 0.2 seconds at 1Mhz clock
    P1OUT &= ~(BIT1);
#else
    P9OUT |= BIT7;
    __delay_cycles(200000); // 0.2 seconds at 1Mhz clock
    P9OUT &= ~(BIT7);
#endif
}

//  P1OUT ^= BIT0;                      // P1.0 = toggle





/*
 * Clears all reset flags (iterates over the generator.)
 *
 * Returns true if one of the reasons is "wake from LPM5"
 * There could be a concurrent other reason.
 *
 * Returns false if none of the reasons is "wake from LPM5."
 * This is usually a cold restart (BOR) but could be the Reset pin or a SW reset (SWBOR)
 *
 * asserts false  for all other reset reasons
 */
bool isResetAWakeFromSleep() {
  bool done = false;
  bool result = false;

  while (! done)
  {
    // not using even_in_range() which is just an optimization, and dissappeared from compiler?
    unsigned int resetReason = SYSRSTIV;
    switch (resetReason)
    {
    case SYSRSTIV_NONE:
      done = true;  // stop loop - all reset reasons are cleared
      break;

    // Expected
    case SYSRSTIV_LPM5WU:
      result = true;
      break;

    // Expected
    case SYSRSTIV_BOR:     // power up
    case SYSRSTIV_RSTNMI:  // RST/NMI pin reset e.g. from debug probe
    case SYSRSTIV_DOBOR:   // software initiated
      break;

    // Security. Accessing BSL that is protected. Probably errant
    case SYSRSTIV_SECYV:     // Security violation
       //assert(false);
       break;

     // WDT Time out
     // Not expected since we stop WD
     case SYSRSTIV_WDTTO:

    // Software initiated POR
    // But our software never initiates.
    case SYSRSTIV_DOPOR:

    // Faults, abnormal e.g. "bus error"
    case SYSRSTIV_UBDIFG:    // FRAM Uncorrectable bit Error

    // Keys
    case SYSRSTIV_WDTKEY:    // WDT Key violation
    case SYSRSTIV_FRCTLPW:   // FRAM Key violation

    case SYSRSTIV_PERF:      // peripheral/config area fetch
    case SYSRSTIV_PMMPW:     // PMM Password violation
#ifdef __MSP430FR2433__
    case SYSRSTIV_FLLUL:     // FLL unlock
#endif
    default:
      assert(false);
      break;
    }
  }
  return result;
}


/*
 * Returns true if any of the reset reasons are not awake from sleep.
 * Awake from sleep may also be a concurrent reason.
 *
 * Returns false if the only reset reason is awake from sleep.
 */
bool isResetNotFromWake() {
  bool done = false;
  bool result = false;

  while (! done)
  {
    // not using even_in_range() which is just an optimization, and dissappeared from compiler?
    unsigned int resetReason = SYSRSTIV;
    switch (resetReason)
    {
    case SYSRSTIV_NONE:
      done = true;  // stop loop - all reset reasons are cleared
      break;

    // Expected
    case SYSRSTIV_LPM5WU:
      break;

    // Expected
    case SYSRSTIV_BOR:     // power up
    case SYSRSTIV_RSTNMI:  // RST/NMI pin reset e.g. from debug probe
    case SYSRSTIV_DOBOR:   // software initiated
      result = true;
      break;

    // Security. Accessing BSL that is protected. Probably errant
    case SYSRSTIV_SECYV:     // Security violation
       //assert(false);
       break;

     // WDT Time out
     // Not expected since we stop WD
     case SYSRSTIV_WDTTO:

    // Software initiated POR
    // But our software never initiates.
    case SYSRSTIV_DOPOR:

    // Faults, abnormal e.g. "bus error"
    case SYSRSTIV_UBDIFG:    // FRAM Uncorrectable bit Error

    // Keys
    case SYSRSTIV_WDTKEY:    // WDT Key violation
    case SYSRSTIV_FRCTLPW:   // FRAM Key violation

    case SYSRSTIV_PERF:      // peripheral/config area fetch
    case SYSRSTIV_PMMPW:     // PMM Password violation
#ifdef __MSP430FR2433__
    case SYSRSTIV_FLLUL:     // FLL unlock
#endif

    default:
      assert(false);
      break;
    }
  }
  return result;
}






int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop WDT

    // Clear NMI and VMA flags so we don't get an immediate interrupt if one has already occurred.
    SFRIFG1 &= ~(NMIIFG | VMAIFG);

    // vacant memory generate interrupt as well as read and execute funny
    SFRIE1 |= VMAIE;

#ifdef __MSP430FR2433__
    // BSL memory behave as vacant memory
    SYSBSLC |= SYSBSLOFF;


    // RTC is not counting
    assert(RTCCTL == 0);
#endif

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
    // if (SYSRSTIV == SYSRSTIV_LPM5WU)        // MSP430 just woke up from LPMx.5
    //if (isResetAWakeFromSleep())
    if (! isResetNotFromWake())
    {
        // Wake from LPM5

        // Example
        blinkRedLed();

        // Port 1 IFG is set but we clear it later.
    }
    else
    {
        // Device powered up from a cold start.
        blinkGreenLed();
    }

    /*
     * Enable a source of a wake interrupt.
     */
    enableButtonInterrupt();    // also clears the IFG before enabling


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
    //_low_power_mode_3();

    /*
     * If we get here, we failed to enter LPM4.5.
     * If we did entern LPM4.5, the continuation is a reset.
     */
    assert(false);

}


/*
 * catch unintended interrupts
 */



/*
 * We don't expect any interrupt ISRs to be called.
 *
 * Even the button interrupt is not called (although we enable it.)
 * See the design for waking from sleep.
 *
 * 27-41 for FR6989
 */
#pragma vector=   \
27, 28, 29, 30, 31, 32, \
33, 34, 35, 36, 37, 38, \
39, 40, 41, \
42, 43, 44, 45, 46, \
47, 48, 49, 50, 51, 52, \
53, 54, 55, 56, 57, 58
__interrupt void ISR_TRAP(void)
{
    blinkGreenLed();
    // Since we did not clear the IFG, this will reoccur??
    while(true) ;
}

