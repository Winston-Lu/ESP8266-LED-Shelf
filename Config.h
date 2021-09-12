#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h> //Some stuff like "progmem" and "byte" wont work without this

//How many LEDS per segment
#define LEDS_PER_LINE 9
#define MILLI_AMPS 5500 // Set the max milli-Amps of your power supply 

//Clock format
#define _12_HR_CLOCK
//#define _24_HR_CLOCK

#define LIGHT_SENSOR  17 //A0
#define DATAPIN       15 //15 = D8 on a NodeMCU
#define SPOTLIGHTPIN  4 //4 = D2 on a NodeMCU. Uncomment if you use a seperate pin for spotlights
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB

#define NAME "LEDShelf"

#define EEPROM_UPDATE_DELAY 15 //in seconds so we dont update constantly to prolong EEPROM lifespan. We will write to EEPROM after this many seconds if a change has been made

extern byte FRAMES_PER_SECOND;  // here you can control the speed. With the Access Point / Web Server the animations run a bit slower.
                                // Originally constant, but can be changed using commands

//Assume direction is from top left to bottom right. Negative if pointing in opposite direction
//numbers here are what the wiring maps to the abstracted array below. Index is what LED segment they are. Add +1 to give 0 a +- sign
//Wiring Index
#ifdef _12_HR_CLOCK
  //1-indexed
  const int PROGMEM segmentWiringOrder[] = {-7,1,8,-14,20,27,-21,15,-9,-2,3,10,-16,22,-28,29,-23,17,-11,-4,5,12,-18,24,-30,31,-25,19,-13,-6,26,-32}; 
  const int PROGMEM spotlightWiringOrder[] = {0,1,2,3,4,5,11,10,9,8,7,6};
  //0-indexed
  const int PROGMEM hyphenSegment = 15;
  //indexes of the digits (0-indexed)
  const int PROGMEM m_one[] = {5,11,12,18,24,25,31}; 
  const int PROGMEM m_ten[] = {3,9,10,16,22,23,29};
  const int PROGMEM h_one[] = {1,7,8,14,20,21,27};
  const int PROGMEM h_ten[] = {-1,-1,6,-1,-1,19,-1}; //-1 means undefined since 12hr doesnt have this segment
#endif
#ifdef _24_HR_CLOCK
  //1-indexed
  const int PROGMEM segmentWiringOrder[] = {-8,1,9,-16,23,31,-24,17,-10,-2,3,11,-18,25,-32,33,-26,19,-12,-4,5,13,-20,27,-34,35,-28,21,-14,-6,7,15,-22,29,-36,37,-30}; 
  const int PROGMEM spotlightWiringOrder[] = {0,1,2,3,4,5,6,13,12,11,10,9,8,7};
  //0-indexed
  const int PROGMEM hyphenSegment = 18;
  //indexes of the digits (0-indexed)
  const int PROGMEM m_one[] = {6,13,14,21,28,29,36}; 
  const int PROGMEM m_ten[] = {4,11,12,19,26,27,34};
  const int PROGMEM h_one[] = {2,9,10,17,24,25,32};
  const int PROGMEM h_ten[] = {0,7,8,15,22,23,30};
#endif
/* 1-Index reference for segmentWiringOrder. 
12hr
  __1_  __2_  __3_  __4_  __5_  __6_ 
_7    _8    _9    10    11    12    13
  _14_  _15_  _16_  _17_  _18_  _19_
20    21    22    23    24    25    26
  _27_  _28_  _29_  _30_  _31_  _32_

24hr
  __1_  __2_  __3_  __4_  __5_  __6_  __7_
_8    _9    10    11    12    13    14    15
  _16_  _17_  _18_  _19_  _20_  _21_  _22_
23    24    25    26    27    28    29    30
  _31_  _32_  _33_  _34_  _35_  _36_  _37_
*/

/* Index reference for spotlights.
12hr
  ----  ----  ----  ----  ----  ----  
--  0 --  1 --  2 --  3 --  4 --  5 --
  ----  ----  ----  ----  ----  ----  
--  6 --  7 --  8 --  9 -- 10 -- 11 --
  ----  ----  ----  ----  ----  ----  

24hr
  ----  ----  ----  ----  ----  ----  ----
--  0 --  1 --  2 --  3 --  4 --  5 --  6 --
  ----  ----  ----  ----  ----  ----  ----
--  7 --  8 --  9 -- 10 -- 11 -- 12 -- 13 -- 
  ----  ----  ----  ----  ----  ----  ----
 */
 
/* Index reference for digits index    
12hr
  --0-  --1-  --2-  --3-  --4-  --5-  
-6    -7    -8    -9    10    11    12
  -13-  -14-  -15-  -16-  -17-  -18-  
19    20    21    22    23    24    25
  -26-  -27-  -28-  -29-  -30-  -31-  

24hr
  --0-  --1-  --2-  --3-  --4-  --5-  --6-  
-7    -8    -9    10    11    12    13    14
  -15-  -16-  -17-  -18-  -19-  -20-  -21-  
22    23    24    25    26    27    28    29
  -30-  -31-  -32-  -33-  -34-  -35-  -36-  
*/


//If diffusion is different per segment (sanded down diffusion) or an LED segment is dimmer than usual, compensate here
const byte segmentBrightnessCompensation[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};



#endif

#ifdef _12_HR_CLOCK
  #define WIDTH 6 
  #define HEIGHT 2
#elif  _24_HR_CLOCK
  #define WIDTH 7 
  #define HEIGHT 2
#else
  #warning "Some settings not work, Time format not selected. Define your own WIDTH, HEIGHT, wiring orders, and segments in Config.h if you have custom dimensions"
  #warning "Defaulting to 12 hour format"
  #define WIDTH 6 
  #define HEIGHT 2
  const int PROGMEM segmentWiringOrder[] = {-7,1,8,-14,20,27,-21,15,-9,-2,3,10,-16,22,-28,29,-23,17,-11,-4,5,12,-18,24,-30,31,-25,19,-13,-6,26,-32}; 
  const int PROGMEM spotlightWiringOrder[] = {0,1,2,3,4,5,11,10,9,8,7,6};
  //indexes of the digits
  const int PROGMEM m_one[] = {5,11,12,18,24,25,31}; 
  const int PROGMEM m_ten[] = {3,9,10,16,22,23,29};
  const int PROGMEM h_one[] = {1,7,8,14,20,21,27};
  const int PROGMEM h_ten[] = {-1,-1,6,-1,-1,19,-1}; //-1 means undefined since 12hr doesnt have this segment
#endif
