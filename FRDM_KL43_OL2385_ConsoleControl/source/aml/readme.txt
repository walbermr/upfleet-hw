AML
================================================================================
Analog middleware layer is designed to improve compatibility between KSDK2.0
and S32 SDK mainly by using the same function prototypes. This layer
automatically distinguishes which type of peripheral is used (for example FTM
or TPM as a timer and SPI, DSPI or LPSPI as type of SPI). This layer supports
main functionality of pheripherals only. There can be implemented support
for additional custom peripherals and for custom MCUs using same prototypes.

Versions
================================================================================
Version 1.2.1
SPI: Default bitcount is 8. Source clock has to be set even for S32 SDK.

Version 1.2
Added fixed TMR_AML_Init: Fixed issue with initialization timer if there 
is no channel for allocation with some devices 
(KL25 call assert if there was 0 channels). Problem is solved.

Version 1.1
Added new implemented feature:
Wait

Version 1.0 (initial).
Implemented pheripherals:
Timer - TPM, FTM
SPI - SPI, DSPI
ADC
GPIO
I2C