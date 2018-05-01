/*
 * ks0108_CoreCode.h - User specific configuration for openGLCD library
 *
 * Use this file to set io pins
 * This pinconfig file used when running in Arduino CoreCode mode
 *
 */

#ifndef GLCD_PIN_CONFIG_H
#define GLCD_PIN_CONFIG_H

/*
 * define name for pin configuration
 */
#define glcd_PinConfigName "ks0108-CoreCode"

/*********************************************************/
/*  Configuration for assigning LCD bits to Arduino Pins */
/*********************************************************/

/* Data pin definitions
 */
#define glcdPinData0        PA2
#define glcdPinData1        PA3
#define glcdPinData2        PA4
#define glcdPinData3        PA5
#define glcdPinData4        PA6
#define glcdPinData5        PA7
#define glcdPinData6        PB0
#define glcdPinData7        PB1

/* Arduino pins used for Control
 * default assignment uses the first five analog pins
 */

#define glcdPinCSEL1        PB8
#define glcdPinCSEL2        PB9

#if NBR_CHIP_SELECT_PINS > 2
#define glcdPinCSEL3         3   // third chip select if needed
#endif

#if NBR_CHIP_SELECT_PINS > 3
#define glcdPinCSEL4         2   // fourth chip select if needed
#endif

#define glcdPinRW           PA0
#define glcdPinDI           PC15
#define glcdPinEN           PA1
// Reset  - uncomment the next line if glcd module reset is connected to an Arduino pin
//#define glcdPinRES          A5    // optional s/w Reset control

//#define glcdPinBL	XX // optional backlight control pin controls BL circuit

#endif //GLCD_PIN_CONFIG_H
