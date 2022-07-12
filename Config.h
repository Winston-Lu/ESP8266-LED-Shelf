#ifndef CONFIG_H
#define CONFIG_H
#include <Arduino.h> //Some stuff like "progmem" and "byte" wont work without this

//How many LEDS per segment
#define LEDS_PER_LINE 9
#define MILLI_AMPS 5500 // Set the max milli-Amps of your power supply 

//Enable sacrifice LED. The first LED can sometimes flash inconsistently when WiFi events happen for some reason, so
// enabling this will allow a 'dummy' LED be the sacrifice LED to take in the bad data and provide the sequential
// LED's with valid data
//#define SACRIFICELED //comment out to disable

//Clock format
#define _12_HR_CLOCK
//#define _24_HR_CLOCK

//Show leading zeros in time. Will show 01:25 instead of 1:25 if set to true. 
//If commented out, default is true for 24hr format, false for 12hr format
//#define DISPLAY_ZERO_IN_TENS_DIGIT true

#define LIGHT_SENSOR  17 //A0
#define DATAPIN       15 //15 = D8 on a NodeMCU
//#define SPOTLIGHTPIN  4 //4 = D2 on a NodeMCU. Uncomment if you use a seperate pin for spotlights
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB

#define NAME "LEDShelf"

#define EEPROM_UPDATE_DELAY 15 //in seconds so we dont update constantly to prolong EEPROM lifespan. We will write to EEPROM after this many seconds if a change has been made

//If we need to clear EEPROM. Recommended to clear EEPROM before the first run, comment out after
//#define RESET_EEPROM

extern byte FRAMES_PER_SECOND;  // here you can control the speed. With the Access Point / Web Server the animations run a bit slower.
                                // Originally constant, but can be changed using commands

//autobrightness stuff
#define AUTOBRIGHTNESS_DELAY 1 //seconds per brightness update. Supports decimal values
#define AUTOBRIGHTNESS_SAMPLES 5 //number of samples to average. Must be an integer

/*  Optional Backlight Example:
 *  Width: 11
 *  Height: 3
 *          top left clockwise led0 >> 0  1  2  3  4  5  6  7  8  9  10 << top right counterclockwise led0
 * top left counterclockwise led0 >> 27                                11 << top right clockwise led0
 *                                   26                                12
 *                                   25                                13
 *                                     24 23 22 21 20 19 18 17 16 15 14       */
//#define BACKLIGHT //If you have backlight behind the clock, define here
#ifdef BACKLIGHT
  #define BACKLIGHT_PIN  14  //D5 on a NodeMCU
  //Where the wiring starts
  #define BACKLIGHT_TOP_LEFT //Options: BACKLIGHT_TOP_LEFT, BACKLIGHT_TOP_RIGHT
  //Wiring direction
  #define BACKLIGHT_CLOCKWISE //Options: BACKLIGHT_CLOCKWISE, BACKLIGHT_COUNTERCLOCKWISE
  //Dimensions
  #define BACKLIGHT_WIDTH 120 //Number of horizontal LED's on the top and bottom
  #define BACKLIGHT_HEIGHT 60 //Number of vertical LED's on the left and right side
#endif

//Assume direction is from top left to bottom right. Negative if pointing in opposite direction
//numbers here are what the wiring maps to the abstracted array below. Index is what LED segment they are. Add +1 to give 0 a +- sign
//Wiring Index
#ifdef _12_HR_CLOCK
  //-----The following wiring orders are 1-indexed-----//
  //Wiring order in the README (Only have one of these uncommented)
  const int PROGMEM segmentWiringOrder[] = {-7,1,8,-14,20,27,-21,15,-9,-2,3,10,-16,22,-28,29,-23,17,-11,-4,5,12,-18,24,-30,31,-25,19,-13,-6,26,-32}; 
  //Wiring order used by DIYMachines
  //const int PROGMEM segmentWiringOrder[] = {-13,-6,12,19,26,-32,-25,-11,-4,10,17,24,-30,-23,-9,-2,8,15,22,-28,-21,7,20,1,3,5,14,16,18,27,29,31};

  //Wiring order in the README (Only have one of these uncommented)
  const int PROGMEM spotlightWiringOrder[] = {0,1,2,3,4,5,11,10,9,8,7,6};
  //Wiring order used by DIYMachines
  //const int PROGMEM spotlightWiringOrder[] = {5,4,3,2,1,0,6,7,8,9,10,11};
  //------------------------//

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
  #ifndef DISPLAY_ZERO_IN_TENS_DIGIT
  #define DISPLAY_ZERO_IN_TENS_DIGIT false
  #endif
#elif defined(_24_HR_CLOCK)
  #define WIDTH 7 
  #define HEIGHT 2
  #ifndef DISPLAY_ZERO_IN_TENS_DIGIT
  #define DISPLAY_ZERO_IN_TENS_DIGIT true
  #endif
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
