#include <FastLED.h>
#include <EEPROM.h> //For persistent settings
#include "Lighting.h"
#include "Config.h"
#include "NTPTime.h"
#include "TimedEffects.h"

CRGB off_color = CRGB::Black;
CRGB leds[NUM_LEDS+1]; //array that gets rendered, +1 for sacrifice LED in case its needed
CRGB spotlightLed[WIDTH*HEIGHT+1]; //dedicated spotlight array if on seperate pin, +1 for sacrifice LED in case its needed
CRGB spotlights[WIDTH * HEIGHT]; //array to keep track of spotlight colors that we set on the web server

//default background colors
CRGB h_ten_color;
CRGB h_one_color;
CRGB m_ten_color;
CRGB m_one_color;
CRGB bg;
CRGB bg2;

//default patterns/colors
byte foregroundPattern = 1;
byte backgroundPattern = 1;
byte spotlightPattern =  1;
const char offPattern[] = "off";            //effect 0
const char solidPattern[] = "solid";        //effect 1
const char rainbowPattern[] = "rainbow";    //effect 2
const char gradientPattern[] = "gradient";  //effect 3
const char rainPattern[] = "rain";          //effect 4
const char sparklePattern[] = "sparkle";    //effect 5
const char firePattern[] = "fire";          //effect 6
const char *const effectNames[] PROGMEM = {offPattern, solidPattern, rainbowPattern, gradientPattern, rainPattern, sparklePattern, firePattern};
//The rest is stored in the spotlights[] array

//Needed for effects requiring some 2d-esque grid
struct grid{
  int verticalSegments[HEIGHT * LEDS_PER_LINE][WIDTH + 1];
  /* verticalSegment at these 2d indexes refer to the following LED's (with LEDS_PER_LINE = 2):
   *      - -      - -     - -     - -     - - 
   * [0,0]   [0,1]    [0,2]   [0,3]   [0,4]   [0,5]
   * [1,0]   [1,1]    [1,2]   [1,3]   [1,4]   [1,5]
   *      - -      - -     - -     - -     - - 
   * [2,0]   [2,1]    [2,2]   [2,3]   [2,4]   [2,5]
   * [3,0]   [3,1]    [3,2]   [3,3]   [3,4]   [3,5]
   *      - -      - -     - -     - -     - - 
   */
  int horizontalSegments[HEIGHT+1][WIDTH * LEDS_PER_LINE];
  /* horizontalSegment at these 2d indexes refer to the following LED's  (with LEDS_PER_LINE = 2):
   *   [0,0][0,1]   [0,2][0,3]   [0,4][0,5]   [0,6][0,7]   [0,8][0,9]
   *  -           -            -            -            -            -
   *  -           -            -            -            -            -
   *   [1,0][1,1]   [1,2][1,3]   [1,4][1,5]   [1,6][1,7]   [1,8][1,9]
   *  -           -            -            -            -            -
   *  -           -            -            -            -            -
   *   [2,0][2,1]   [2,2][2,3]   [2,4][2,5]   [2,6][2,7]   [2,8][2,9]
   */

   //Age of raindrop (Time to live) for rain effect
   int verticalLedTTL[HEIGHT*LEDS_PER_LINE][WIDTH+1];
   int horizontalLedTTL[HEIGHT][WIDTH * LEDS_PER_LINE];

   //Height for fire effect
   int fireHeight[WIDTH*LEDS_PER_LINE+1];
} grid2d;

int power = 1;
int hueOffset = 0;
byte segmentBrightness = 77; //0-255
byte spotlightBrightness = 255; //0-255
byte backgroundBrightness = 25;
byte foregroundTransparency = 255; //255 = solid
boolean autobrightness = false;

int rainbowRate = 5;
uint32_t clockRefreshTimer = 0;
uint32_t lastUpdate = 0;
bool updateSettings = false;
bool autoEffect = false;
uint32_t loadingCursorPosition = 0; 
byte hyphenLength = 0;
CRGB hyphenColor = CRGB::Black;

strip stripSegment, convert;
changelist lightingChanges;

uint16_t debugPrintCounter = 0;

/* Clock Segment Index
  # 0 #
  1   2
  # 3 #
  4   5
  # 6 #
*/
//                  x0123456  which segment needs to be up for that number
const int PROGMEM zero  = 0b01110111;
const int PROGMEM one   = 0b00010010;
const int PROGMEM two   = 0b01011101;
const int PROGMEM three = 0b01011011;
const int PROGMEM four  = 0b00111010;
const int PROGMEM five  = 0b01101011;
const int PROGMEM six   = 0b01101111;
const int PROGMEM seven = 0b01010010;
const int PROGMEM eight = 0b01111111;
const int PROGMEM nine  = 0b01111011;

//************************************************//
//           Show Selected Lighting               //
//************************************************//
void showLightingEffects() {
  if(power == 0){
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    #ifdef SPOTLIGHTPIN
    fill_solid(spotlightLed, WIDTH*HEIGHT, CRGB::Black);
    #endif
    return;
  }
  if(autobrightness){
    double val = (analogRead(LIGHT_SENSOR)/1024.0)*2; //Between 0-2
    segmentBrightness = min(max((int)(40.0 * val*val*val),5),150);//Clamp between 5 and 150, median 50
    backgroundBrightness = min(max((int)(12.0 * val*val*val * 0.8),2),100); //Clamp between 2 and 100, median 30
    spotlightBrightness = min(max((int)(80*(val*val*val)*2),30),255); //Clamp between 30 and 255, median 160
  }
  //If we toggle on time-scheduled effects
  if(autoEffect) scheduleLighting();
  //Regular lighting behaviour
  else{
    //Spotlights
    #ifdef SPOTLIGHTPIN // if dedicated spotlight pin
    switch (spotlightPattern) {
      case 0: //off
        solidSpotlightsDedicated(CRGB::Black);break;
      case 1: //Solid
        solidUniqueSpotlightsDedicated();
        applySpotlightBrightnessDedicated();
        break;
      case 2: //rainbow
        rainbowSpotlights();
        applySpotlightBrightnessDedicated();
        break;
      case 3: //gradient
        gradientSpotlights(spotlights[0], spotlights[1]);
        applySpotlightBrightnessDedicated();
        break;
      case 4: //rain
        rain(30, CRGB::Black, spotlights[0]);
        dimSpotlightsDedicated(max(spotlightBrightness / 8, 2));
        break;
      case 5: //sparkle
        sparkle(10 , spotlights[0] , NUM_SEGMENTS * LEDS_PER_LINE , WIDTH * HEIGHT, 255 - spotlightBrightness); //10 chance seems fine. Note this isnt 10% or 10/255% chance. See sparkle() for how the chance works
        dimSpotlightsDedicated(max(spotlightBrightness / 8, 2));
        break;
      case 6: //fire
        fire();
        dimSpotlightsDedicated(max(spotlightBrightness / 8, 2));
        break;
    }
    #else
    switch (spotlightPattern) {
      case 0: //off
        solidSpotlights(CRGB::Black);
        break;
      case 1: //Solid
        solidUniqueSpotlights();
        applySpotlightBrightness();
        break;
      case 2: //rainbow
        rainbowSpotlights();
        applySpotlightBrightness();
        break;
      case 3: //gradient
        gradientSpotlights(spotlights[0], spotlights[1]);
        applySpotlightBrightness();
        break;
      case 4: //rain
        rain(30, CRGB::Black, spotlights[0]);
        dimSpotlights(max(spotlightBrightness / 8, 2));
        break;
      case 5: //sparkle
        sparkle(10 , spotlights[0] , NUM_SEGMENTS * LEDS_PER_LINE , WIDTH * HEIGHT, 255 - spotlightBrightness); //10 chance seems fine. Note this isnt 10% or 10/255% chance. See sparkle() for how the chance works
        dimSpotlights(max(spotlightBrightness / 8, 2));
        break;
      case 6: //fire
        fire();
        dimSpotlights(max(spotlightBrightness / 8, 2));
        break;
    }
    #endif
    //Background
    switch (backgroundPattern) {
      case 0: //off
        solidSegments(CRGB::Black); break;
      case 1: //solid
        solidSegments(bg); 
        dimSegments(255 - backgroundBrightness);
        break;
      case 2: //rainbow
        for (int i = 0; i < NUM_SEGMENTS; i++) rainbowSegment(i, segmentLightingOffset(i)*LEDS_PER_LINE * rainbowRate, rainbowRate);
        dimSegments(255 - backgroundBrightness);
        break;
      case 3: //gradient
        for (int i = 0; i < NUM_SEGMENTS; i++) gradientSegment(i, bg, bg2);
        dimSegments(255 - backgroundBrightness);
        break;
      case 4: //rain
        rain(15, bg, CRGB::Black);
        dimSegments(max(backgroundBrightness / 10, 50));
        break;
      case 5: //sparkle
        sparkle(100 , bg , 0 , NUM_SEGMENTS * LEDS_PER_LINE, 255 - backgroundBrightness); //100 chance seems fine. Note this isnt 100% or 100/255% chance. See sparkle() for how the chance works
        dimSegments(max(backgroundBrightness / 10, 2));
        break;
      case 6: //fire
        if(spotlightPattern != 6) fire(); //call the effect if it hasnt been called already
        dimSegments(max(backgroundBrightness / 8, 2));
        break;
      case 255: //Loading effect from manual config
        loadingEffect(bg);
        dimSpotlights(50);
        break;
    }
    //Foreground
    switch (foregroundPattern) {
      case 0:
        break;//do nothing. Just here to acknowledge this option exists
      case 1: //solid
        if (clockRefreshTimer == FRAMES_PER_SECOND * 3) { updateTime();clockRefreshTimer = 0;}
        #ifdef _12_HR_CLOCK
        render_clock_to_display(getHour12(), getMinute(), 255 - segmentBrightness);
        #elif defined(_24_HR_CLOCK)
        render_clock_to_display(getHour24(), getMinute(), 255 - segmentBrightness);
        #endif
        clockRefreshTimer++;
        break;
      case 2: //rainbow
        if (clockRefreshTimer == FRAMES_PER_SECOND * 3) { updateTime();clockRefreshTimer = 0;}
        #ifdef _12_HR_CLOCK
        render_clock_to_display_rainbow(getHour12(), getMinute(), 255 - segmentBrightness);
        #elif defined(_24_HR_CLOCK)
        render_clock_to_display_rainbow(getHour24(), getMinute(), 255 - segmentBrightness);
        #endif
        clockRefreshTimer++;
        break;
      case 3: //gradient
        if (clockRefreshTimer == FRAMES_PER_SECOND * 3) { updateTime();clockRefreshTimer = 0;}
        #ifdef _12_HR_CLOCK
        render_clock_to_display_gradient(getHour12(), getMinute(), 255 - segmentBrightness);
        #elif defined(_24_HR_CLOCK)
        render_clock_to_display_gradient(getHour24(), getMinute(), 255 - segmentBrightness);
        #endif
        clockRefreshTimer++;
        break;
    }
  }
  //Hyphen segment if enabled
  if(hyphenColor.r != 0 || hyphenColor.g != 0 || hyphenColor.b != 0){
    strip seg = segmentToLedIndex(hyphenSegment);
    //If gap is odd, round the leftLedSkip up.since typically the m_ten values will be further right (for digits 1 & 7)
    //Hyphen length of 4 = 3left -hyphen- 2right. Clamp values to 0-LEDS_PER_LINE
    const int leftLedSkip = min(max(0,(LEDS_PER_LINE-hyphenLength)/2 + (LEDS_PER_LINE-hyphenLength)%2),LEDS_PER_LINE); 
    if(seg.reverse){
      for(int i=0; i<leftLedSkip+hyphenLength ; i++){
        if(i<leftLedSkip) continue;
        leds[seg.start-i] = hyphenColor;
      }
    }else{
      for(int i=0; i<leftLedSkip+hyphenLength ; i++){
        if(i<leftLedSkip) continue;
        leds[seg.start+i] = hyphenColor;
      }
    }
  }
  
  //Change rainbow color
  hueOffset += rainbowRate;
  
  //Compensate for different segment diffusion if it is configured/exists
  for(int i=0;i<sizeof(segmentBrightnessCompensation)/sizeof(segmentBrightnessCompensation[0]);i++)
    if(segmentBrightnessCompensation[i]!=0) dimSegment(i,segmentBrightnessCompensation[i]);
    
}

//************************************************//
//              Display Functions                 //
//************************************************//

void clearDisplay() {                     fill_solid(leds, NUM_LEDS, CRGB::Black); }
void applySegmentBrightness() {           fadeToBlackBy(leds                                        , NUM_SEGMENTS * LEDS_PER_LINE     , 255 - segmentBrightness  );}
void applySpotlightBrightness() {         fadeToBlackBy(&leds[NUM_SEGMENTS * LEDS_PER_LINE]         , WIDTH * HEIGHT                   , 255 - spotlightBrightness);}
void applySpotlightBrightnessDedicated() {fadeToBlackBy(spotlightLed                                , WIDTH * HEIGHT                   , 255 - spotlightBrightness);}
void dimSegments(byte val) {              fadeToBlackBy(leds                                        , NUM_SEGMENTS * LEDS_PER_LINE     , val);}
void dimSpotlights(byte val) {            fadeToBlackBy(&leds[NUM_SEGMENTS * LEDS_PER_LINE]         , WIDTH * HEIGHT                   , val);}
void dimSpotlightsDedicated(byte val){    fadeToBlackBy(spotlightLed                                , WIDTH * HEIGHT                   , val);}
void dimLed(int index, byte val) {        fadeToBlackBy(&leds[index]                                , 1                                , val);}
void dimSegment(int segment, byte val) {
  if (segment == -1) return;
  strip segmentStruct = segmentToLedIndex(segment);
  if (segmentStruct.reverse)       fadeToBlackBy(&leds[segmentStruct.start - LEDS_PER_LINE + 1], LEDS_PER_LINE                   , val);
  else                             fadeToBlackBy(&leds[segmentStruct.start]                    , LEDS_PER_LINE                   , val);
}

//************************************************//
//                  Clock Effect                  //
//************************************************//
void render_clock_to_display(int h, int m) {render_clock_to_display(h, m, 0);}
void render_clock_to_display(int h, int m, byte dim) {
  const uint8_t light_tens_h = sevenSegment(h / 10);
  const uint8_t light_ones_h = sevenSegment(h % 10);
  const uint8_t light_tens_m = sevenSegment(m / 10);
  const uint8_t light_ones_m = sevenSegment(m % 10);
  //Set the digits we want to the color
  for (int i = 0; i < 7; i++) {
    //Change hours tens LEDS
    if (light_tens_h & 0b01000000 >> i && h/10) { //use bitmask to see if the segment is supposed to be on for that digit
      CRGB segmentColor = dimColor(h_ten_color, dim);
      addSegmentColor(h_ten[i], segmentColor, foregroundTransparency);
    }
    //Change hours ones LEDS
    if (light_ones_h & 0b01000000 >> i) {
      CRGB segmentColor = dimColor(h_one_color, dim);
      addSegmentColor(h_one[i], segmentColor, foregroundTransparency);
    }
    //Change minutes tens LEDS
    if (light_tens_m & 0b01000000 >> i) {
      CRGB segmentColor = dimColor(m_ten_color, dim);
      addSegmentColor(m_ten[i], segmentColor, foregroundTransparency);
    }
    //Change minutes ones LEDS
    if (light_ones_m & 0b01000000 >> i) {
      CRGB segmentColor = dimColor(m_one_color, dim);
      addSegmentColor(m_one[i], segmentColor, foregroundTransparency);
    }
  }
}

void render_clock_to_display_rainbow(int h, int m) {render_clock_to_display_rainbow(h, m, 0);}
void render_clock_to_display_rainbow(int h, int m, byte dim) {
  const uint8_t light_tens_h = sevenSegment(h / 10);
  const uint8_t light_ones_h = sevenSegment(h % 10);
  const uint8_t light_tens_m = sevenSegment(m / 10);
  const uint8_t light_ones_m = sevenSegment(m % 10);
  //Set the digits we want to the color
  for (int i = 0; i < 7; i++) {
    //Change hours tens LEDS
    if (light_tens_h & 0b01000000 >> i && h/10 == 1) { //use bitmask to see if the segment is supposed to be on for that digit
      rainbowSegment(h_ten[i], (byte)segmentLightingOffset(h_ten[i])*rainbowRate * LEDS_PER_LINE, rainbowRate, foregroundTransparency);
      dimSegment(h_ten[i], dim);
    }
    //Change hours ones LEDS
    if (light_ones_h & 0b01000000 >> i) {
      rainbowSegment(h_one[i], (byte)segmentLightingOffset(h_one[i])*rainbowRate * LEDS_PER_LINE, rainbowRate, foregroundTransparency);
      dimSegment(h_one[i], dim);
    }
    //Change minutes tens LEDS
    if (light_tens_m & 0b01000000 >> i) {
      rainbowSegment(m_ten[i], (byte)segmentLightingOffset(m_ten[i])*rainbowRate * LEDS_PER_LINE, rainbowRate, foregroundTransparency);
      dimSegment(m_ten[i], dim);
    }
    //Change minutes ones LEDS
    if (light_ones_m & 0b01000000 >> i) {
      rainbowSegment(m_one[i], (byte)segmentLightingOffset(m_one[i])*rainbowRate * LEDS_PER_LINE, rainbowRate, foregroundTransparency);
      dimSegment(m_one[i], dim);
    }
  }
}

void render_clock_to_display_gradient(int h, int m) {render_clock_to_display_gradient(h, m, 0);}
void render_clock_to_display_gradient(int h, int m, byte dim) {
  const uint8_t light_tens_h = sevenSegment(h / 10);
  const uint8_t light_ones_h = sevenSegment(h % 10);
  const uint8_t light_tens_m = sevenSegment(m / 10);
  const uint8_t light_ones_m = sevenSegment(m % 10);
  //Set the digits we want to the color
  for (int i = 0; i < 7; i++) {
    //Change hours tens LEDS
    if (light_tens_h & 0b01000000 >> i && h/10 == 1) { //use bitmask to see if the segment is supposed to be on for that digit
      gradientSegment(h_ten[i], h_ten_color, h_one_color, foregroundTransparency);
      dimSegment(h_ten[i], dim);
    }
    //Change hours ones LEDS
    if (light_ones_h & 0b01000000 >> i) {
      gradientSegment(h_one[i], h_ten_color, h_one_color, foregroundTransparency);
      dimSegment(h_one[i], dim);
    }
    //Change minutes tens LEDS
    if (light_tens_m & 0b01000000 >> i) {
      gradientSegment(m_ten[i], h_ten_color, h_one_color, foregroundTransparency);
      dimSegment(m_ten[i], dim);
    }
    //Change minutes ones LEDS
    if (light_ones_m & 0b01000000 >> i) {
      gradientSegment(m_one[i], h_ten_color, h_one_color, foregroundTransparency);
      dimSegment(m_one[i], dim);
    }
  }
}

//************************************************//
//                 Color Effects                  //
//************************************************//

void setSegmentColor(int segment, CRGB color) {
  stripSegment = segmentToLedIndex(segment); //convert from abstract segment index to wiring LED positions
  if (segment == -1) return;
  if (stripSegment.reverse && stripSegment.start != -1) {
    for (int i = 0; i < LEDS_PER_LINE; i++) {
      leds[stripSegment.start - i] = color;
    }
  } else {
    for (int i = 0; i < LEDS_PER_LINE; i++) {
      leds[stripSegment.start + i] = color;
    }
  }
}

void addSegmentColor(int segment, CRGB color, byte transparency) {
  stripSegment = segmentToLedIndex(segment); //convert from abstract segment index to wiring LED positions
  if (segment == -1) return;
  if (stripSegment.reverse && stripSegment.start != -1) {
    for (int i = 0; i < LEDS_PER_LINE; i++) {
      CRGB newColor;
      newColor.red   = (byte)(color.red  * (transparency / 255.0) + leds[stripSegment.start - i].red  * (1 - (transparency / 255.0)));
      newColor.green = (byte)(color.green * (transparency / 255.0) + leds[stripSegment.start - i].green * (1 - (transparency / 255.0)));
      newColor.blue  = (byte)(color.blue * (transparency / 255.0) + leds[stripSegment.start - i].blue * (1 - (transparency / 255.0)));
      leds[stripSegment.start - i] = newColor;
    }
  } else {
    for (int i = 0; i < LEDS_PER_LINE; i++) {
      CRGB newColor;
      newColor.red   = (byte)(color.red  * (transparency / 255.0) + leds[stripSegment.start + i].red  * (1 - (transparency / 255.0)));
      newColor.green = (byte)(color.green * (transparency / 255.0) + leds[stripSegment.start + i].green * (1 - (transparency / 255.0)));
      newColor.blue  = (byte)(color.blue * (transparency / 255.0) + leds[stripSegment.start + i].blue * (1 - (transparency / 255.0)));
      leds[stripSegment.start + i] = newColor;
    }
  }
}

void setSpotlightColor(int index, CRGB color) {
  spotlights[index] = color;
}
void solidSegments(CRGB color) {
  for (int i = 0; i < NUM_SEGMENTS*LEDS_PER_LINE; i++)
    leds[i] = color;
}
void solidSpotlights(CRGB color) {
  for (int i = 0; i < WIDTH * HEIGHT; i++)
    leds[LEDS_PER_LINE * NUM_SEGMENTS + i ] = color;
}
void solidSpotlightsDedicated(CRGB color) {
  for (int i = 0; i < WIDTH * HEIGHT; i++)
    spotlightLed[i] = color;
}
void solidUniqueSpotlights(){
  for (int i = 0; i < WIDTH * HEIGHT; i++) 
    spotlightLed[spotlightToLedIndexDedicated(i)] = spotlights[i]; //Set spotlight color to the dedicated array of spotlights
}
void solidUniqueSpotlightsDedicated(){
  for (int i = 0; i < WIDTH * HEIGHT; i++) 
    leds[spotlightToLedIndex(i)] = spotlights[i]; //Set spotlight color to the dedicated array of spotlights
}

void gradientSegment(int segment, CRGB color1, CRGB color2) {gradientSegment(segment, color1, color2, 255);}
void gradientSegment(int segment, CRGB color1, CRGB color2, byte transparency) {
  const int maxLedVal = (WIDTH + HEIGHT) * LEDS_PER_LINE; //max value to use for division
  const float increment = 1.0 / (maxLedVal); //The change in brightness for each consecutive LED
  const int offset = segmentLightingOffset(segment); //the segment offset brightness
  strip segmentStruct = segmentToLedIndex(segment);

  for (int i = 0; i < LEDS_PER_LINE; i++) {
    //                    color2 is from 0->100%                                  color1 is from 100->0%
    //                    depending on the segmentLightingOffset, color2 increases and color1 increases linearly to create a gradient
    CRGB color;
    //                                    totalLedOffset          *increment                      100%      -       totalLedOffset       *increment
    color.red   = (byte)(( color2.red  * (offset * LEDS_PER_LINE + i) * increment ) +  ( color1.red  * (maxLedVal - (offset * LEDS_PER_LINE + i)) * increment ));
    color.green = (byte)(( color2.green * (offset * LEDS_PER_LINE + i) * increment ) +  ( color1.green * (maxLedVal - (offset * LEDS_PER_LINE + i)) * increment ));
    color.blue  = (byte)(( color2.blue * (offset * LEDS_PER_LINE + i) * increment ) +  ( color1.blue * (maxLedVal - (offset * LEDS_PER_LINE + i)) * increment ));
    if (segmentStruct.reverse) {
      leds[segmentStruct.start - i].r = color.r*(transparency/255.0) + leds[segmentStruct.start - i].r*(1-(transparency/255.0));
      leds[segmentStruct.start - i].g = color.g*(transparency/255.0) + leds[segmentStruct.start - i].g*(1-(transparency/255.0));
      leds[segmentStruct.start - i].b = color.b*(transparency/255.0) + leds[segmentStruct.start - i].b*(1-(transparency/255.0));
    } else {
      leds[segmentStruct.start + i].r = color.r*(transparency/255.0) + leds[segmentStruct.start + i].r*(1-(transparency/255.0));
      leds[segmentStruct.start + i].g = color.g*(transparency/255.0) + leds[segmentStruct.start + i].g*(1-(transparency/255.0));
      leds[segmentStruct.start + i].b = color.b*(transparency/255.0) + leds[segmentStruct.start + i].b*(1-(transparency/255.0));
    }
  }
}
void gradientSpotlights(CRGB color1, CRGB color2) {
  //start at color offset 1 instead of 0. I offset by 1 since the spotlights blend between segments with offset n and offset n+1, so the midpoint color would be LEDS_PER_LINE*offset*(n+1)
  //cause color with offset 'n' would have color offsets: LEDS_PER_LINE*offset*n -> LEDS_PER_LINE*offset*(n+1), and the other segment would have offsets from LEDS_PER_LINE*offset*(n+1) -> LEDS_PER_LINE*offset*(n+2)
  /*
      -- -- -- -- -- --
     | 0| 1| 2| 3| 4| 5|
      -- -- -- -- -- --
     | 1| 2| 3| 4| 5| 6|
      -- -- -- -- -- --
  */
  const int maxLedVal = WIDTH * HEIGHT; //max value to use for division
  const float increment = 1.0 / maxLedVal; //The change in brightness for each consecutive segment

  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    const int offset = i % WIDTH + i / WIDTH;
    CRGB color;
    //                    color2 is from 0->100%                    color1 is from 100->0%
    //                    depending on the segmentLightingOffset, color2 increases and color1 increases linearly to create a gradient
    color.red   = (byte)(( color2.red    * increment * (offset + 1)) +  (color1.red    *  ( increment * (maxLedVal - offset - 2 ) ) ));
    color.green = (byte)(( color2.green  * increment * (offset + 1)) +  (color1.green  *  ( increment * (maxLedVal - offset - 2 ) ) ));
    color.blue  = (byte)(( color2.blue   * increment * (offset + 1)) +  (color1.blue   *  ( increment * (maxLedVal - offset - 2 ) ) ));
    #ifdef SPOTLIGHTPIN
    spotlightLed[spotlightToLedIndexDedicated(i)] = color;
    #else
    leds[spotlightToLedIndex(i)] = color;
    #endif
  }
}

void rainbowSegment(int segment, uint8_t offset, uint8_t rate) {rainbowSegment(segment,offset,rate,255);}
void rainbowSegment(int segment, uint8_t offset, uint8_t rate, byte transparency){
  stripSegment = segmentToLedIndex(segment); //convert from abstract segment index to wiring LED positions
  if (stripSegment.reverse) {
    stripSegment.start -= (LEDS_PER_LINE - 1); //we need the lowest LED index since the fill_rainbow counts up
    offset += rate * LEDS_PER_LINE; //moving in opposite direction, switch offset from min->max to max->min, Since its reverse, instead of starting hue at 0 then moving to 10, we should start at 10, then move to 0
    rate = -rate; //moving in opposite direction, reverse hue. Probably not good practice to modify the parameter variable
  }
  //Save the old background colors since fill_rainbow does not save transparency
  CRGB oldColors[LEDS_PER_LINE];
  for (int i = 0; i < LEDS_PER_LINE; i++) {
    oldColors[i] = leds[stripSegment.start + i];
  }

  //Fill segment with rainbow
  //          start of LED segment      # of LEDS to change   segment offset + hue color reffect    rate of color change
  fill_rainbow(&leds[stripSegment.start], LEDS_PER_LINE,       offset + hueOffset,                     rate);
  //Note that offset already contains the lightingOffset value depending on the location of the segment

  //Add back the background color
  for (int i = 0; i < LEDS_PER_LINE; i++) {
    CRGB newColor;
    newColor.red   = leds[stripSegment.start + i].red   * (foregroundTransparency / 255.0) + oldColors[i].red   * (1 - (foregroundTransparency / 255.0));
    newColor.green = leds[stripSegment.start + i].green * (foregroundTransparency / 255.0) + oldColors[i].green * (1 - (foregroundTransparency / 255.0));
    newColor.blue  = leds[stripSegment.start + i].blue  * (foregroundTransparency / 255.0) + oldColors[i].blue  * (1 - (foregroundTransparency / 255.0));
    leds[stripSegment.start + i] = newColor;
  }
}

void rainbowSpotlights(){
  //This rainbow hue color is taken from the middle of the top/left segment, rather than the average/median between the 4 surronding segments.
  //If you want to change this to be the average/median of the 4 surrounding segments, get rid of the '/2' at the end of the totalHueOffset variable: "rainbowRate*LEDS_PER_LINE/2"
  for(int y=0 ; y<HEIGHT ; y++){
    for(int x=0 ; x<WIDTH ; x++){
      //                current color | segmentOffset*amount of color changed/segment | 
      byte totalHueOffset = hueOffset + (x+y)*rainbowRate*LEDS_PER_LINE               + rainbowRate*LEDS_PER_LINE/2;

      #ifdef SPOTLIGHTPIN
      fill_rainbow(&spotlightLed[spotlightToLedIndexDedicated(y*WIDTH+x)], 1,    totalHueOffset ,          0);
      #else
      fill_rainbow(&leds[spotlightToLedIndex(y*WIDTH+x)], 1,    totalHueOffset ,          0);
      //            spotlight LED index              # of LEDS    Hue offset        Delta (since its only 1 LED, 0 is fine)
      #endif
    }
  }
}

void sparkle(int chance, int ledStart, int len) {             sparkle(chance, CHSV(random8(), 255, 255), ledStart, len, 0);}
void sparkle(int chance, CRGB color, int ledStart, int len) { sparkle(chance, color,                     ledStart, len, 0);}
void sparkle(int chance, CRGB color, int ledStart, int len, byte dim) {
  while (1) {
    byte randomVal = random8(128);
    //give a chance to spawn multiple sparkles in 1 run. That way we arent limited to a 0-100% chance each iteration.
    //Depending on the chance, it can be anywhere from 0-12800% chance to spawn, where each 100% guarantees 1 sparkle. With a 255 chance, average of ~4 particles spawn per iteration, which should be enough.
    //If you have a really large array of LED's and you want to increase this, change random8(128) to a lower value like random8(64) for a ~8 particle spawn per iteration
    if (chance > randomVal) {
      uint16_t randNum = random16(len); //pick random index within the specified range
      chance -= randomVal; //decrement counter so we could spawn more than 1 per iteration
      leds[ledStart + randNum] = color;
      dimLed(ledStart + randNum, dim); //Apply initial brightness/dim LED when spawned
      #ifdef SPOTLIGHTPIN
      if(ledStart + randNum > LEDS_PER_LINE*NUM_SEGMENTS){
        spotlightLed[(ledStart + randNum) % (WIDTH*HEIGHT)] = color;
        fadeToBlackBy(&spotlightLed[(ledStart + randNum) % (WIDTH*HEIGHT)], 1, dim);
      }
      #endif
    } else break;
  }
}

void rain(byte chance, CRGB color, CRGB spotlightColor) {
  const int verticalLedChanceMultiplier = 5;
  //Run if we set segment effect to a non-black color
  if(backgroundPattern == 4 && (color.r!=0 || color.g !=0 || color.b !=0)){
    //Horizontal segment effect
    //Calculate falling from a Time-To-Live (TTL) array
    for(int y=0 ; y<HEIGHT ; y++){
      for(int x=0 ; x<WIDTH*LEDS_PER_LINE ; x++){
        if(grid2d.horizontalLedTTL[y][x] == 1){
          //Dont set a TTL on the last row, since it cant fall after that and it would be out-of-bounds
          if(y<HEIGHT-1) grid2d.horizontalLedTTL[y+1][x] = LEDS_PER_LINE+1;
          leds[grid2d.horizontalSegments[y+1][x]] = color;
        }
        grid2d.horizontalLedTTL[y][x]--;
      }
    }
    
    //Vertical segment effect
    //Calculate falling from a Time-To-Live (TTL) array
    for(int y=HEIGHT*LEDS_PER_LINE-2 ; y >= 0 ; y--){
      for(int x=0 ; x < WIDTH+1 ; x++){
        grid2d.verticalLedTTL[y][x]--;
        if(grid2d.verticalLedTTL[y][x] == LEDS_PER_LINE){
          grid2d.verticalLedTTL[y+1][x] = LEDS_PER_LINE+1; //can be any number really, the number above should be this number - 1
          leds[grid2d.verticalSegments[y+1][x]] = color;
        }
      }
    }

    //Spawn new raindrops on segments
    while (1) {
      byte randomVal = random8(128);
      //give a chance to spawn multiple raindrops in 1 run. That way we arent limited to a 0-100% chance each iteration.
      //Depending on the chance, it can be anywhere from 0-12800% chance to spawn, where each 100% guarantees 1 sparkle. With a 255 chance, average of ~4 particles spawn per iteration, which should be enough.
      //If you have a really large array of LED's and you want to increase this, change random8(128) to a lower value like random8(64) for a ~8 particle spawn per iteration
      if (chance > randomVal) {
        uint16_t randNum = random16(WIDTH*LEDS_PER_LINE + verticalLedChanceMultiplier*(WIDTH+1)); //pick random index within the specified range. The WIDTH+1 is for the vertical segments
        //Spawn on a horizontal segment
        if(randNum < WIDTH*LEDS_PER_LINE){
          grid2d.horizontalLedTTL[0][randNum] = LEDS_PER_LINE+1;
          leds[grid2d.horizontalSegments[0][randNum]] = color;
        }
        //Spawn on a vertical segment
        else{ 
          grid2d.verticalLedTTL[0][(randNum - WIDTH*LEDS_PER_LINE)/verticalLedChanceMultiplier] = LEDS_PER_LINE+1;
          leds[grid2d.verticalSegments[0][(randNum - WIDTH*LEDS_PER_LINE)/verticalLedChanceMultiplier]] = color;
        }
        chance -= randomVal; //decrement counter so we could spawn more than 1 per iteration
      } else break;
    }
  }
  //if background not selected but spotlights are, generate a map of raindrops but dont render it
  else if(backgroundPattern != 4 && (spotlightColor.r!=0 || spotlightColor.g !=0 || spotlightColor.b !=0)){
    //standalone spotlight effect if background rain is not selected
    //Use the above code for generating rain on horizontal segments, but dont render the segments
    for(int y=0 ; y<HEIGHT ; y++){
      for(int x=0 ; x<WIDTH*LEDS_PER_LINE ; x++){
        if(grid2d.horizontalLedTTL[y][x] == 1){
          if(y<HEIGHT-1){
            grid2d.horizontalLedTTL[y+1][x] = LEDS_PER_LINE+1;
            leds[grid2d.horizontalSegments[y+1][x]] = spotlightColor;
          }
        }
        grid2d.horizontalLedTTL[y][x]--;
      }
    }
    while (1) {
      byte randomVal = random8(128);
      float chanceOffset = (float)(WIDTH*verticalLedChanceMultiplier)/(WIDTH*(LEDS_PER_LINE+1)+1);
      if (chance*chanceOffset > randomVal) {
        uint16_t randNum = random16(WIDTH*LEDS_PER_LINE);
        grid2d.horizontalLedTTL[0][randNum] = LEDS_PER_LINE+1;
        leds[grid2d.horizontalSegments[0][randNum]] = spotlightColor;
        chance -= randomVal; //decrement counter so we could spawn more than 1 per iteration
      } else break;
    }
  }
  
  //Calculate spotlight falling based on horizontal TTL segments
  if(spotlightPattern == 4 && (spotlightColor.r!=0 || spotlightColor.g !=0 || spotlightColor.b !=0)){
    for(int y=0 ; y < HEIGHT ; y++){
      for(int x=0 ; x < WIDTH ; x++){
        //Get average TTL for each segment above a spotlight
        float avgTTL = 0;
        for(int i=0 ; i<LEDS_PER_LINE ; i++)
          avgTTL += max(grid2d.horizontalLedTTL[y][x*LEDS_PER_LINE+i]-1,0);
        CRGB newColor;
        newColor.r = spotlightColor.r * (avgTTL/(LEDS_PER_LINE+1)/LEDS_PER_LINE);
        newColor.g = spotlightColor.g * (avgTTL/(LEDS_PER_LINE+1)/LEDS_PER_LINE);
        newColor.b = spotlightColor.b * (avgTTL/(LEDS_PER_LINE+1)/LEDS_PER_LINE);
        #ifdef SPOTLIGHTPIN
        spotlightLed[spotlightToLedIndexDedicated(y*WIDTH + x)] = newColor ;
        #else
        leds[spotlightToLedIndex(y*WIDTH + x)] = newColor ;
        #endif
      }
    }
  }
}

void fire(){
  const int fireChance = 20;
  const int minHeight = HEIGHT*LEDS_PER_LINE*0.2;
  const int effectHeight = HEIGHT*LEDS_PER_LINE;
  const int maxWidth = WIDTH*LEDS_PER_LINE;
  /* generate random vals for fireHeight[], and create a 5-wide flame
   |
   |   |       |        |
   |  |||     ||    |   ||
   |_|||||_|__|||__||__||||_
   */
  for(int i=0 ; i<maxWidth+1 ; i++){
    //If it has not been initialized or we want to randomly initialize it
    if(grid2d.fireHeight[i] == 0) grid2d.fireHeight[i] = max(1,minHeight)-1+random8(3);
    if(fireChance > random8()){
      //Create a kind of parabolic flame effect
      const int maxHeight         = minHeight + random8(effectHeight-minHeight);
      const int heightDifference1 = maxHeight         - (effectHeight*0.1 + random8((byte)(effectHeight*0.1)));
      const int heightDifference2 = heightDifference1 - (effectHeight*0.2 + random8((byte)(effectHeight*0.2)));

      //Clamp x-values between 0 and WIDTH*LEDS_PER_LINE and create new Y values if greater. 
      grid2d.fireHeight[max(0,i-2)]        = max(heightDifference2 - random8(2),grid2d.fireHeight[max(0,i-2)]);
      grid2d.fireHeight[max(0,i-1)]        = max(heightDifference1 - random8(2),grid2d.fireHeight[max(0,i-1)]);
      grid2d.fireHeight[i]                 = max(maxHeight                     ,grid2d.fireHeight[i]);
      grid2d.fireHeight[min(maxWidth,i+1)] = max(heightDifference1 - random8(2),grid2d.fireHeight[min(maxWidth,i+1)]);
      grid2d.fireHeight[min(maxWidth,i+2)] = max(heightDifference2 - random8(2),grid2d.fireHeight[min(maxWidth,i+2)]);
    }else{
      //Calm down the flame a bit
      grid2d.fireHeight[i] = max(minHeight-1+random8(3), grid2d.fireHeight[i]-(1+random8((byte)(effectHeight*0.05))));
    }
  }
  //render on spotlights. 
  if(spotlightPattern == 6){
    for(int x=0;x<WIDTH;x++){
      //calculate average flame height
      int avgHeight = 0;
      for(int j=0;j<LEDS_PER_LINE+1;j++){
        avgHeight = grid2d.fireHeight[x*LEDS_PER_LINE+j];
      }
      //set spotlight position to LEDS_PER_LINE/2 + verticalOffset
      int spotlightHeight = LEDS_PER_LINE/2;
      CRGB color;
      for(int y=0;y<HEIGHT;y++){
        //Get gradient
        if(spotlightHeight-LEDS_PER_LINE/2 > avgHeight){ //if average is below that segment, dim
          fadeToBlackBy(&leds[spotlightToLedIndex((HEIGHT-1-y)*WIDTH + x)], 1 , 160);
        }else if(spotlightHeight > avgHeight){ //if spotlight is above average height of flames, set a point between that and black
          color.r = spotlights[0].r*(1-(float)(spotlightHeight-avgHeight)/LEDS_PER_LINE);
          color.g = spotlights[0].g*(1-(float)(spotlightHeight-avgHeight)/LEDS_PER_LINE);
          color.b = spotlights[0].b*(1-(float)(spotlightHeight-avgHeight)/LEDS_PER_LINE);
        }else{  //if average is above the spotlight position, get gradient
          color.r = spotlights[0].r*((float)spotlightHeight/avgHeight) + spotlights[1].r*(1 - ((float)spotlightHeight/avgHeight));
          color.g = spotlights[0].g*((float)spotlightHeight/avgHeight) + spotlights[1].g*(1 - ((float)spotlightHeight/avgHeight));
          color.b = spotlights[0].b*((float)spotlightHeight/avgHeight) + spotlights[1].b*(1 - ((float)spotlightHeight/avgHeight));
        }
        spotlightHeight += LEDS_PER_LINE;
        #ifdef SPOTLIGHTPIN
        spotlightLed[spotlightToLedIndexDedicated((HEIGHT-1-y)*WIDTH + x)] = color;
        #else
        leds[spotlightToLedIndex((HEIGHT-1-y)*WIDTH + x)] = color;
        #endif
      }
    }
  }
  //render on background
  if(backgroundPattern == 6){
    for(int x=0;x<maxWidth+1;x++){
      //We added 1 extra LED for the right-most vertical segment, which would go out of bounds for our horizontal segments
      //If on a horizontal segment
      if(x!=maxWidth){
        for(int h=0 ; h<=HEIGHT ; h++){ 
          int ledNum = grid2d.horizontalSegments[HEIGHT-h][x];
          if(grid2d.fireHeight[x] >= h*LEDS_PER_LINE - (h!=0)){ //the h!=0 since it goes 0,8,17
            float randomColorOffset = random16(grid2d.fireHeight[x]) * h==0; //so the bottom row isnt static
            float gradientOffset = min((float)1,max((float)0,((float)(h*LEDS_PER_LINE+randomColorOffset)) /  ((float)(grid2d.fireHeight[x]))));
            leds[ledNum] = CRGB(bg.r*(gradientOffset) + bg2.r*(1 - gradientOffset),
                                bg.g*(gradientOffset) + bg2.g*(1 - gradientOffset),
                                bg.b*(gradientOffset) + bg2.b*(1 - gradientOffset)  );
          }else fadeToBlackBy(&leds[ledNum], 1 , 120);
        }
      }
    }
    //Vertical segments
    for(int x=0;x<WIDTH+1;x++){
      //Gradient the segments
      for(int y=0;y<min(HEIGHT*LEDS_PER_LINE,grid2d.fireHeight[x]);y++){
        int ledNum = grid2d.verticalSegments[HEIGHT*LEDS_PER_LINE-y-1][x];
        leds[ledNum] = CRGB(bg.r*((float)y/grid2d.fireHeight[x]) + bg2.r*(1 - ((float)y/grid2d.fireHeight[x])),
                            bg.g*((float)y/grid2d.fireHeight[x]) + bg2.g*(1 - ((float)y/grid2d.fireHeight[x])),
                            bg.b*((float)y/grid2d.fireHeight[x]) + bg2.b*(1 - ((float)y/grid2d.fireHeight[x])));
      }
      //Otherwise dim those segments
      for(int y=grid2d.fireHeight[x] ; y<HEIGHT*LEDS_PER_LINE ; y++){
        fadeToBlackBy(&leds[grid2d.verticalSegments[HEIGHT*LEDS_PER_LINE-y-1][x]], 1 , 120);
      }
    }
    
  }
}

void loadingEffect(CRGB color){
  if       (loadingCursorPosition <    WIDTH        *LEDS_PER_LINE){  //top segments
    leds[grid2d.horizontalSegments[0][loadingCursorPosition]] = color;
  }else if (loadingCursorPosition < (  WIDTH+HEIGHT)*LEDS_PER_LINE){  //right segments
    leds[grid2d.verticalSegments[loadingCursorPosition - WIDTH*LEDS_PER_LINE][WIDTH]] = color;
  }else if (loadingCursorPosition < (2*WIDTH+HEIGHT)*LEDS_PER_LINE){  //bottom segments
    leds[grid2d.horizontalSegments[HEIGHT][(2*WIDTH+HEIGHT)*LEDS_PER_LINE-loadingCursorPosition-1]] = color;
  }else if (loadingCursorPosition < 2*(WIDTH+HEIGHT)*LEDS_PER_LINE){  //left segments
    leds[grid2d.verticalSegments[2*(WIDTH+HEIGHT)*LEDS_PER_LINE-loadingCursorPosition-1][0]] = color;
  }else{
    loadingCursorPosition = -1;
  }
  loadingCursorPosition++;
  dimSegments(30);
}

//************************************************//
//                Helper Functions                //
//************************************************//
//Change color offset, where top-left is 0 and bottom right is the maximum value so we can get a hue/color gradient
/* Segment color reference
  --0-  --1-  --2-  --3-  --4-  --5-
-0    -1    -2    -3    -4    -5    -6
  --1-  --2-  --3-  --4-  --5-  --6-
-1    -2    -3    -4    -5    -6    -7
  --2-  --3-  --4-  --5-  --6-  --7-
*/
byte segmentLightingOffset(int index) {
  const int rowLength = (2 * WIDTH + 1);
  const int row = index / rowLength; //every set of vertical and horizontal segments is a row
  const int indexOffset = index % rowLength;
  if (indexOffset < WIDTH) { //if horizontal segment
    return (row + indexOffset); //since each consecutive row's lighting offset starts at the row #, just add the horizontal index position after
  } else { //if vertical segment
    return (row + indexOffset - WIDTH); //since each consecutive row's lighting offset starts at the row #, just add the horizontal index position after
  }
}

strip segmentToLedIndex(int index) {
  //map from the abstracted segment index to the actual wiring index
  //go through entire segmentWiringOrder array
  for (int i = 0; i < sizeof(segmentWiringOrder) / sizeof(int); i++) {
    //If at the segmentWiringOrder index, we find the index of the abstracted segment index, return the wiring position
    if (abs(segmentWiringOrder[i]) - 1 == index) { //subtract 1 since we start at 1 (0 cant be given a positive or negative sign)
      convert.start = i * LEDS_PER_LINE;
      if (segmentWiringOrder[i] < 0) { //if negative, the LED direction is opposite of the direction the LED effects go (top right to bottom left)
        convert.reverse = true;
        convert.start += LEDS_PER_LINE - 1;
      } else {
        convert.reverse = false;
      }
      return (convert);
    }
  }
  //If not found, return an invalid strip struct
  convert.start = -1;
  convert.reverse = false;
  return (convert);
}

int spotlightToLedIndex(int index) {
  for(int i=0;i<WIDTH*HEIGHT;i++){
    if(spotlightWiringOrder[i] == index) return(NUM_SEGMENTS * LEDS_PER_LINE + i);
  }
  return -1;
  
  //Assume zig-zag wiring (Works for wiring that I did, but this algoritm is hard coded)
//  if ((index / WIDTH) % 2 == 0) { //forward direction
//    //        spotlight offset         +   vertical offset   +  index
//    return (NUM_SEGMENTS * LEDS_PER_LINE + (index / WIDTH) * WIDTH + index);
//  } else { //reverse direction
//    //        spotlight offset         +   vertical offset   +   reverse index
//    return (NUM_SEGMENTS * LEDS_PER_LINE + (index / WIDTH) * WIDTH + WIDTH - index % WIDTH - 1);
//  }

}
int spotlightToLedIndexDedicated(int index) {
  for(int i=0;i<WIDTH*HEIGHT;i++){
    if(spotlightWiringOrder[i] == index) return(i);
  }
  return -1;
}

CRGB dimColor(CRGB color, byte amount) {
  CRGB newColor;
  newColor.red   = (byte)(color.red  * (1 - amount / 255.0));
  newColor.green = (byte)(color.green * (1 - amount / 255.0));
  newColor.blue  = (byte)(color.blue * (1 - amount / 255.0));
  return newColor;
}

uint8_t sevenSegment(int num) {
  switch (num) {
    case 0: return (zero);
    case 1: return (one);
    case 2: return (two);
    case 3: return (three);
    case 4: return (four);
    case 5: return (five);
    case 6: return (six);
    case 7: return (seven);
    case 8: return (eight);
    case 9: return (nine);
  }
  return 0;
}
void storeUtcOffset(double value){
  EEPROM.begin(512);
  int16_t toStore = (int16_t)value*60; //store in minutes
  EEPROM.write(1,toStore>>8);
  EEPROM.write(2,toStore%256);
  EEPROM.commit();
}

double getUtcOffset(){
  EEPROM.begin(512);
  int16_t ans = EEPROM.read(1)*256 + EEPROM.read(2);
  Serial.println("UTC Offset: " + String(ans/60));
  return (ans/60.0);
}
void saveAllSettings(){
  lightingChanges.power = true;
  lightingChanges.foregroundTransparency = true;
  lightingChanges.autobrightness = true;
  lightingChanges.segmentBrightness = true;
  lightingChanges.spotlightBrightness = true;
  lightingChanges.backgroundBrightness = true;
  lightingChanges.h_ten_color = true;
  lightingChanges.h_one_color = true;
  lightingChanges.m_ten_color = true;
  lightingChanges.m_one_color = true;
  lightingChanges.bg = true;
  lightingChanges.bg2 = true;
  for(int i=0;i<WIDTH*HEIGHT;i++){
    lightingChanges.spotlights[i] = true;
  }
  lightingChanges.foregroundPattern = true;
  lightingChanges.backgroundPattern = true;
  lightingChanges.spotlightPattern = true;
  lightingChanges.rainbowRate = true;
  lightingChanges.fps = true;
  lightingChanges.hyphenLength = true;
  lightingChanges.hyphenColor = true;
  lastUpdate = 0;
  updateSettings = false;
  storeEEPROM();
}

void clearLightingCache(){
  //Clear TTL cache
  for(int y=0;y<HEIGHT;y++){
    for(int x=0;x<WIDTH*LEDS_PER_LINE;x++){
      grid2d.horizontalLedTTL[y][x] = 0;
    }
  }
  for(int y=0;y<HEIGHT*LEDS_PER_LINE;y++){
    for(int x=0;x<WIDTH+1;x++){
      grid2d.verticalLedTTL[y][x] = 0;
    }
  }
  //Clear fire cache
  for(int i=0 ; i < WIDTH*LEDS_PER_LINE+1 ; i++){
    grid2d.fireHeight[i] = 0;
  }
  //Clear screen segments (spotlights dont need to be wiped)
  solidSegments(CRGB::Black);
}

String getCurrentSettings(String seperator){
  String ans = "Power: " + String(power ? "On" : "Off") + seperator;
  ans +=  "Foreground Transparency: " + String(foregroundTransparency*100/255) + "%" + seperator;
  ans +=  "Autobrightness: " + String(autobrightness ? "On" : "Off") + seperator;
  ans +=  "Segment Brightness: " + String(segmentBrightness*100/255) + "%" + seperator;
  ans +=  "Background Brightness: " + String(backgroundBrightness*100/255) + "%" + seperator;
  ans +=  "Spotlight Brightness: " + String(backgroundBrightness*100/255) + "%" + seperator;
  ans +=  "h_ten_color: " + String(crgbToCss(h_ten_color)) + seperator;
  ans +=  "h_one_color: " + String(crgbToCss(h_one_color)) + seperator;
  ans +=  "m_ten_color: " + String(crgbToCss(m_ten_color)) + seperator;
  ans +=  "m_one_color: " + String(crgbToCss(m_one_color)) + seperator;
  ans +=  "bg: " + String(crgbToCss(bg)) + seperator;
  ans +=  "bg2: " + String(crgbToCss(bg2)) + seperator;
  for(int i=0;i<WIDTH*HEIGHT;i++){
    ans += "spotlight " + String(i) + ": " + String(crgbToCss(spotlights[i])) + seperator;
  }
  ans +=  "Foreground pattern: " + String(effectNames[foregroundPattern]) + seperator;
  ans +=  "Background pattern: " + String(effectNames[backgroundPattern]) + seperator;
  ans +=  "Spotlight pattern: "  + String(effectNames[spotlightPattern]) + seperator;
  ans +=  "Rainbow rate: "  + String(rainbowRate) + seperator;
  ans +=  "fps: "  + String(FRAMES_PER_SECOND) + seperator;
  ans +=  "Hyphen Length: "  + String(hyphenLength) + seperator;
  ans +=  "Hyphen Color: "  + String(crgbToCss(hyphenColor));
  ans +=  "UTC offset: " + String(getUtcOffset());
  return ans;
}

void shiftLedsByOne(){
  #ifdef SPOTLIGHTPIN
    for(int i=NUM_LEDS - (WIDTH*HEIGHT); i > 0 ;i--)
      leds[i] = leds[i-1];
    leds[0] = CRGB::Black;
    for(int i=WIDTH*HEIGHT; i>0 ;i--)
      spotlightLed[i] = spotlightLed[i-1];
    spotlightLed[0] = CRGB::Black;
  #else
    for(int i=NUM_LEDS; i > 0 ;i--)
      leds[i] = leds[i-1];
    leds[0] = CRGB::Black;
  #endif
}

int timeToMinutes(int h, int m){return h*60 + m;}

//************************************************//
//             WebServer Functions                //
//************************************************//
//Runs when site is loaded
String parseSettings() {
  Serial.println("Getting settings");
  String settings = "";
  settings += String(power) + "|";
  settings += String(foregroundTransparency) + "|";
  settings += String(autobrightness) + "|";
  settings += String(segmentBrightness) + "|";
  settings += String(spotlightBrightness) + "|";
  settings += String(backgroundBrightness) + "|";
  settings += crgbToCss(h_ten_color) + '|';
  settings += crgbToCss(h_one_color) + '|';
  settings += crgbToCss(m_ten_color) + '|';
  settings += crgbToCss(m_one_color) + '|';
  settings += crgbToCss(bg) + '|';
  settings += crgbToCss(bg2) + '|';
  settings += crgbToCss(spotlights[0]) + '|';
  settings += crgbToCss(spotlights[1]) + '|';
  settings += crgbToCss(spotlights[2]) + '|';
  settings += crgbToCss(spotlights[3]) + '|';
  settings += crgbToCss(spotlights[4]) + '|';
  settings += crgbToCss(spotlights[5]) + '|';
  settings += crgbToCss(spotlights[6]) + '|';
  settings += crgbToCss(spotlights[7]) + '|';
  settings += crgbToCss(spotlights[8]) + '|';
  settings += crgbToCss(spotlights[9]) + '|';
  settings += crgbToCss(spotlights[10]) + '|';
  settings += crgbToCss(spotlights[11]) + '|';
  Serial.println("Getting selected pattern in settings");
  Serial.println("FG: " + String(effectNames[foregroundPattern]) + " (" + String(foregroundPattern) + "/" + String(sizeof(effectNames)/sizeof(effectNames[0])) + 
               ") BG: " + String(effectNames[backgroundPattern]) + " (" + String(foregroundPattern) + "/" + String(sizeof(effectNames)/sizeof(effectNames[0])) + 
               ") SL: " + String(effectNames[spotlightPattern])  + " (" + String(foregroundPattern) + "/" + String(sizeof(effectNames)/sizeof(effectNames[0])) + ")");
               
  if(foregroundPattern >= sizeof(effectNames)/sizeof(effectNames[0]) || foregroundPattern < 0){
    Serial.println("Critical error: Foreground pattern number invalid: " + String(foregroundPattern));
    settings += String(effectNames[1]); //default case
  }else settings += String(effectNames[foregroundPattern]);
  settings += '|';
  
  if(backgroundPattern >= sizeof(effectNames)/sizeof(effectNames[0]) || backgroundPattern < 0){
    Serial.println("Critical error: Background pattern number invalid: " + String(backgroundPattern));
    settings += String(effectNames[1]); //default case
  }else settings += String(effectNames[backgroundPattern]);
  settings += '|';
  
  if(spotlightPattern >= sizeof(effectNames)/sizeof(effectNames[0]) || spotlightPattern < 0){
    Serial.println("Critical error: Spotlightground pattern number invalid: " + String(spotlightPattern));
    settings += String(effectNames[1]); //default case
  }else settings += String(effectNames[spotlightPattern]);
  
  Serial.println("Done getting settings");
  return settings;
}
void setSegmentBrightness(byte brightness)    {segmentBrightness    = brightness;}
void setSpotlightBrightness(byte brightness)  {spotlightBrightness  = brightness;}
void setBackgroundBrightness(byte brightness) {backgroundBrightness = brightness;}
void setForegroundTransparency(byte trans)    {foregroundTransparency = trans;}
void setSpotlight1(CRGB color) {spotlights[0] = color;}
void setSpotlight2(CRGB color) {spotlights[1] = color;}
String crgbToCss(CRGB val) {return ( "#" + byteToHexString(val.red) + byteToHexString(val.green) + byteToHexString(val.blue));}
String byteToHexString(byte num){return ( numToHex(num / 16) + numToHex(num % 16));}
String numToHex(byte num) {
  switch (num) {
    case 0 ... 9:
      return String(num);
    case 10:
      return "a";
    case 11:
      return "b";
    case 12:
      return "c";
    case 13:
      return "d";
    case 14:
      return "e";
    case 15:
      return "f";
  }
  return "0";
}
void deleteSettings(){EEPROM.begin(512); EEPROM.write(0,0); EEPROM.commit();}

//************************************************//
//              Initialize variables              //
//************************************************//
void fastLEDInit(){
  #ifdef SPOTLIGHTPIN
  FastLED.addLeds<LED_TYPE, DATAPIN, COLOR_ORDER>(leds, NUM_LEDS - (WIDTH*HEIGHT) + 1).setCorrection(TypicalLEDStrip); 
  FastLED.addLeds<LED_TYPE, SPOTLIGHTPIN, COLOR_ORDER>(spotlightLed, WIDTH*HEIGHT+1).setCorrection(TypicalLEDStrip);
  #else
  FastLED.addLeds<LED_TYPE, DATAPIN, COLOR_ORDER>(leds, NUM_LEDS+1).setCorrection(TypicalLEDStrip); 
  #endif
}

void defaultSettings(){
  Serial.println("Setting options to default settings");
  power = 1;
  foregroundTransparency = 255;
  autobrightness = false;
  segmentBrightness = 77;
  backgroundBrightness = 38;
  spotlightBrightness = 255;
  h_ten_color = CRGB::White;
  h_one_color = CRGB::White;
  m_ten_color = CRGB::Cyan;
  m_one_color = CRGB::Cyan;
  bg = CRGB::Gray;
  bg2 = CRGB::White;
  for(int i=0;i<WIDTH*HEIGHT;i++){
    leds[spotlightToLedIndex(i)] = CRGB::White;
    spotlights[i] = CRGB::White;
  }
  foregroundPattern = 1;
  backgroundPattern = 1;
  spotlightPattern = 1;
  rainbowRate = 5;
  FRAMES_PER_SECOND = 30;
  hyphenLength = 0;
  hyphenColor = CRGB::Black;
}

void loadEEPROM(){
  Serial.println("Loading EEPROM");
  EEPROM.begin(512);
  byte addr = 3; //first byte is checking if data is written, next 2 bytes are for utcOffset
  byte val = 0;
  //power
  power = EEPROM.read(addr++);
  //clock transparency
  foregroundTransparency = EEPROM.read(addr++);
  //auto brightness
  autobrightness = EEPROM.read(addr++) == 1;
  //clock brightness
  segmentBrightness = EEPROM.read(addr++);  
  //background brightness
  backgroundBrightness = EEPROM.read(addr++);  
  //spotlight brightness
  spotlightBrightness = EEPROM.read(addr++);  
  //hours 10 color
  h_ten_color = CRGB(EEPROM.read(addr),EEPROM.read(addr+1),EEPROM.read(addr+2));  addr+=3;
  //hours 1 color
  h_one_color = CRGB(EEPROM.read(addr),EEPROM.read(addr+1),EEPROM.read(addr+2));  addr+=3;
  //minutes 10 color
  m_ten_color = CRGB(EEPROM.read(addr),EEPROM.read(addr+1),EEPROM.read(addr+2));  addr+=3;
  //minutes 1 color
  m_one_color = CRGB(EEPROM.read(addr),EEPROM.read(addr+1),EEPROM.read(addr+2));  addr+=3;
  //bg
  bg = CRGB(EEPROM.read(addr),EEPROM.read(addr+1),EEPROM.read(addr+2));  addr+=3;
  //bg2
  bg2 = CRGB(EEPROM.read(addr),EEPROM.read(addr+1),EEPROM.read(addr+2));  addr+=3;
  //spotlight patterns
  for(int i=0;i<WIDTH*HEIGHT;i++){
    spotlights[i] = CRGB(EEPROM.read(addr),EEPROM.read(addr+1),EEPROM.read(addr+2));
    addr+=3;
  }
  //foreground pattern
  foregroundPattern = EEPROM.read(addr++);  
  //background pattern
  backgroundPattern = EEPROM.read(addr++);  
  //spotlight patterns
  spotlightPattern = EEPROM.read(addr++);
  //rainbow speed
  rainbowRate = EEPROM.read(addr++);
  //fps
  FRAMES_PER_SECOND = max((byte)5,EEPROM.read(addr++)); //if 0, causes eternal reset loop. This is in case this value doesnt get
  //hyphenLength
  hyphenLength = EEPROM.read(addr++);
  //hyphenColor
  hyphenColor = CRGB(EEPROM.read(addr),EEPROM.read(addr+1),EEPROM.read(addr+2));  addr+=3;
  Serial.println("Done loading from EEPROM. Loaded " + String(addr-3) + " bytes up to 0x" + byteToHexString(addr));
}

void storeEEPROM(){
  Serial.println("Storing to EEPROM:");
  EEPROM.begin(512);
  int addr = 3;
  bool madeChanges = false;
  if(lightingChanges.power)                     {EEPROM.write(addr,power);                                                                                                  madeChanges = true;}     addr++;
  if(lightingChanges.foregroundTransparency)    {EEPROM.write(addr,foregroundTransparency);                                                                                 madeChanges = true;}     addr++;
  if(lightingChanges.autobrightness)            {EEPROM.write(addr,autobrightness);                                                                                         madeChanges = true;}     addr++;
  if(lightingChanges.segmentBrightness)         {EEPROM.write(addr,segmentBrightness);                                                                                      madeChanges = true;}     addr++;
  if(lightingChanges.backgroundBrightness)      {EEPROM.write(addr,backgroundBrightness);                                                                                   madeChanges = true;}     addr++;
  if(lightingChanges.spotlightBrightness)       {EEPROM.write(addr,spotlightBrightness);                                                                                    madeChanges = true;}     addr++;
  if(lightingChanges.h_ten_color)               {EEPROM.write(addr,h_ten_color.red);    EEPROM.write(addr+1,h_ten_color.green);   EEPROM.write(addr+2,h_ten_color.blue);    madeChanges = true;}     addr+=3;
  if(lightingChanges.h_one_color)               {EEPROM.write(addr,h_one_color.red);    EEPROM.write(addr+1,h_one_color.green);   EEPROM.write(addr+2,h_one_color.blue);    madeChanges = true;}     addr+=3;
  if(lightingChanges.m_ten_color)               {EEPROM.write(addr,m_ten_color.red);    EEPROM.write(addr+1,m_ten_color.green);   EEPROM.write(addr+2,m_ten_color.blue);    madeChanges = true;}     addr+=3;
  if(lightingChanges.m_one_color)               {EEPROM.write(addr,m_one_color.red);    EEPROM.write(addr+1,m_one_color.green);   EEPROM.write(addr+2,m_one_color.blue);    madeChanges = true;}     addr+=3;
  if(lightingChanges.bg)                        {EEPROM.write(addr,bg.red);             EEPROM.write(addr+1,bg.green);            EEPROM.write(addr+2,bg.blue);             madeChanges = true;}     addr+=3;
  if(lightingChanges.bg2)                       {EEPROM.write(addr,bg2.red);            EEPROM.write(addr+1,bg2.green);           EEPROM.write(addr+2,bg2.blue);            madeChanges = true;}     addr+=3;
  for(int i=0;i<WIDTH*HEIGHT;i++){
    if(lightingChanges.spotlights[i])           {EEPROM.write(addr,spotlights[i].red);  EEPROM.write(addr+1,spotlights[i].green); EEPROM.write(addr+2,spotlights[i].blue);  madeChanges = true;}     addr+=3;
  }
  if(lightingChanges.foregroundPattern)         {EEPROM.write(addr,foregroundPattern);                                                                                      madeChanges = true;}     addr++;
  if(lightingChanges.backgroundPattern)         {EEPROM.write(addr,backgroundPattern);                                                                                      madeChanges = true;}     addr++;
  if(lightingChanges.spotlightPattern)          {EEPROM.write(addr,spotlightPattern);                                                                                       madeChanges = true;}     addr++;
  if(lightingChanges.rainbowRate)               {EEPROM.write(addr,rainbowRate);                                                                                            madeChanges = true;}     addr++;
  if(lightingChanges.fps)                       {EEPROM.write(addr,FRAMES_PER_SECOND%256);                                                                                  madeChanges = true;}     addr++;
  if(lightingChanges.hyphenLength)              {EEPROM.write(addr,hyphenLength);                                                                                           madeChanges = true;}     addr++;
  if(lightingChanges.hyphenColor)               {EEPROM.write(addr,hyphenColor.red);    EEPROM.write(addr+1,hyphenColor.green);   EEPROM.write(addr+2,hyphenColor.blue);    madeChanges = true;}     addr+=3;
  if(!madeChanges) return;
  EEPROM.write(0,1);
  EEPROM.commit();
  Serial.println("Done storing to EEPROM up to 0x" + byteToHexString(addr));
}

void lightingInit() {
  loadingCursorPosition = random16(2*(WIDTH+HEIGHT)*LEDS_PER_LINE);
  EEPROM.begin(512);
  if(EEPROM.read(0)==1){
    loadEEPROM();
  }
  else{
    //Load from EEPROM if it exists
    defaultSettings();
    saveAllSettings();
  }

  
  //init grid struct
  int vertSegX = 0;
  int vertSegY = 0;
  int horzSegX = 0;
  int horzSegY = 0;
  for (int i = 0; i < NUM_SEGMENTS; i++) {
    strip seg = segmentToLedIndex(i);
    //Determine if segment is horizontal or vertical
    /* Reference
      --0-  --1-  --2-  --3-  --4-  --5-  
    -6    -7    -8    -9    10    11    12
      -13-  -14-  -15-  -16-  -17-  -18-  
    19    20    21    22    23    24    25
      -26-  -27-  -28-  -29-  -30-  -31-  
    */
    //Since a row repeats every 13 segments (or 2*WIDTH+1 segments), horizontal is when i%13 < 6, otherwise vertical

    if( i%(2*WIDTH+1) < WIDTH){  //Horizontal
      if(seg.reverse)
        for(int i=0;i<LEDS_PER_LINE;i++)  grid2d.horizontalSegments[horzSegY][horzSegX*LEDS_PER_LINE + i] = seg.start - i; //Reverse x-direction
      else
        for(int i=0;i<LEDS_PER_LINE;i++)  grid2d.horizontalSegments[horzSegY][horzSegX*LEDS_PER_LINE + i] = seg.start + i; //Forward x-direction
      //Move to next row if reached end
      if(++horzSegX == WIDTH){
        horzSegX = 0;
        horzSegY++;
      }
    }else{     //Vertical
      if(seg.reverse)
        for(int i=0;i<LEDS_PER_LINE;i++)  grid2d.verticalSegments[vertSegY*LEDS_PER_LINE + i][vertSegX] = seg.start - i; //Reverse y-direction
      else
        for(int i=0;i<LEDS_PER_LINE;i++)  grid2d.verticalSegments[vertSegY*LEDS_PER_LINE + i][vertSegX] = seg.start + i; //Forward y-direction
      //Move to next row if reached end
      if(++vertSegX == WIDTH+1){
        vertSegX = 0;
        vertSegY++;
      }
    }
  }
  //Set to 0's in case memory doesnt already have it at 0
  clearLightingCache(); 
  Serial.println("Done initializing lighting data");
}
