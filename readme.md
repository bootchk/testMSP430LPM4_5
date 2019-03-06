
Example code for MSP430 Low Power Mode 4.5


About
-----

For demonstrating and testing basic elements of the MSP430 Low Power Mode 4.5 (LPM4.5).

LPM4.5 is unusual and hard to wrap your head around.

There are many examples provided by TI, but I couldn't find a full example, just pieces, say to wake up once.
Also their examples are sparsely commented.
This example wakes up many times, and has comments about unusual and obscure points.

Also, I wanted to explore the difference between CCS "debug" mode (active JTAG) and "Free Run" mode (with JTAG disabled),
especially with regard to using EnergyTrace.

I also wanted to explore the limitations of EnergyTrace in extreme low power modes.
The resolution of EnergyTrace is .001mW (or about 300nA) which is much above the power consumed in LPM4.5 
(as low as 16nA, depends on many factors, LPM4.5 is a whole class of modes, or minor configurations of the system).
So you can't use EnergyTrace to confirm that you are actually using the least power that you possible can with the MSP430.


Specifics
---------

    - As coded, is specific to the EXP-MSP430FR2433 LaunchPad dev kit.
    - I used the free, TI provided Code Composer Studio (CCS v8), an IDE base on Eclipse.
    - Language is C.


Basic behaviour
---------------

First burn the program to the target.

When not using the IDE/debugger: when you power up the LaunchPad (plug in the USB), the green LED (P1.1) of the target should blink, representing cold start condition.  
Then the program should sleep in LPM4.5 consuming low power.  If you press the button on P2.3, the program should wake and blink the red LED (P1.0.)

When using the IDE/debugger and you choose "Resume" icon: same behaviour.

When using the IDE/debugger and you choose "Run>Free Run" menu item: the behaviour is different??? 
The green LED lights, but the red LED does not light when you press the button, although an EnergyTrace power curves seems to show that the system IS waking up when you push the button.


Using EnergyTrace, you should see a relatively high pulse when the program starts and when you push the button (the energy for the active cpu and to light the LED.)
Except when you use "Free Run."




