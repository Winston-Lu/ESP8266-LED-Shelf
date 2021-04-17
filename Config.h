#ifndef CONFIG_H
#define CONFIG_H
#define FASTLED_INTERNAL
#include <ESP8266WiFi.h>

//How many LEDS per segment
#define LEDS_PER_LINE 9
#define MILLI_AMPS 5500 // Set the max milli-Amps of your power supply 
#define FRAMES_PER_SECOND  30  // here you can control the speed. With the Access Point / Web Server the animations run a bit slower.

//Clock format
#define _12_HR_CLOCK
//#define _24_HR_CLOCK

#define LIGHT_SENSOR  17 //A0
#define DATAPIN       15
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB

#define NAME "LEDShelf"

#define EEPROM_UPDATE_DELAY 5 //in seconds so we dont update constantly to prolong EEPROM lifespan. We will write to EEPROM after this many seconds if a change has been made





//Assume direction is from top left to bottom right. Negative if pointing in opposite direction
//numbers here are what the wiring maps to the abstracted array below. Index is what LED segment they are. Add +1 to give 0 a +- sign
//Wiring Index
const int PROGMEM segmentWiringOrder[] = {-7,1,8,-14,20,27,-21,15,-9,-2,3,10,-16,22,-28,29,-23,17,-11,-4,5,12,-18,24,-30,31,-25,19,-13,-6,26,-32}; 
const int PROGMEM spotlightWiringOrder[] = {0,1,2,3,4,5,11,10,9,8,7,6};
/* Index reference for segmentWiringOrder. spotlightWiringOrder startsat 0 since it doesnt need a sign for direction
  --1-  --2-  --3-  --4-  --5-  --6- 
-7    -8    -9    10    11    12    13
  -14-  -15-  -16-  -17-  -18-  -19-
20    21    22    23    24    25    26
  -27-  -28-  -29-  -30-  -31-  -32-
*/

//indexes of the digits
const int PROGMEM m_one[] = {5,11,12,18,24,25,31}; 
const int PROGMEM m_ten[] = {3,9,10,16,22,23,29};
const int PROGMEM h_one[] = {1,7,8,14,20,21,27};
const int PROGMEM h_ten[] = {-1,-1,6,-1,-1,19,-1};
/* Digit index    
  --0-  --1-  --2-  --3-  --4-  --5- 
-6    -7    -8    -9    10    11    12
  -13-  -14-  -15-  -16-  -17-  -18- 
19    20    21    22    23    24    25
  -26-  -27-  -28-  -29-  -30-  -31- 
*/

#endif

#ifdef _12_HR_CLOCK
  #define WIDTH 6 
  #define HEIGHT 2
#elif  _24_HR_CLOCK
  #define WIDTH 7 
  #define HEIGHT 2
#else
  #warning "Clock may not work, Time format not selected. Change WIDTH and HEIGHT in Config.h if you have custom dimensions"
  #define WIDTH 6 
  #define HEIGHT 2
#endif
