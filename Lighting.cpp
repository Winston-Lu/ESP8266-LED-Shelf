#include <FastLED.h>
#include <EEPROM.h> //For persistent settings
#include "Lighting.h"
#include "Config.h"
#include "NTPTime.h"

CRGB off_color = CRGB::Black;
CRGB leds[NUM_LEDS]; //array that gets rendered
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
const char offPattern[] PROGMEM = "off";            //effect 0
const char solidPattern[] PROGMEM = "solid";        //effect 1
const char rainbowPattern[] PROGMEM = "rainbow";    //effect 2
const char gradientPattern[] PROGMEM = "gradient";  //effect 3
const char rainPattern[] PROGMEM = "rain";          //effect 4
const char sparklePattern[] PROGMEM = "sparkle";    //effect 5
const char *const effectNames[] PROGMEM = {offPattern, solidPattern, rainbowPattern, gradientPattern, rainPattern, sparklePattern};
//The rest is stored in the spotlights[] array

//Needed for rain effect
int verticalSegments[HEIGHT * LEDS_PER_LINE][WIDTH + 1];

int power = 1;
int hueOffset = 0;
byte segmentBrightness = 77; //0-255
byte spotlightBrightness = 255; //0-255
byte backgroundBrightness = 25;
byte foregroundTransparency = 255; //255 = solid
boolean autobrightness = false;

int rainbowRate = 5;
uint32_t clockRefreshTimer = 0;
bool firstRun = true;
uint32_t lastUpdate = 0;
bool updateSettings = false;

strip stripSegment, convert;
changelist lightingChanges;

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
//foregroundPatterns = {"off","solid","rainbow","gradient"};
//backgroundPatterns = {"off","solid","rainbow","gradient","rain","sparkle"};
//spotlightPatterns = {"off","solid","rainbow","gradient","rain","sparkle"};
void showLightingEffects() {
  if (firstRun) {
    firstRun = false;
  }
  if(autobrightness){
    double val = (analogRead(LIGHT_SENSOR)/1024.0)*2; //Between 0-2
    segmentBrightness = min(max((int)(50.0 * val*val*val),5),150);//Clamp between 5 and 150, median 50
    backgroundBrightness = min(max((int)(30.0 * val*val*val * 0.8),2),100); //Clamp between 2 and 100, median 30
    spotlightBrightness = min(max((int)(80*(val*val*val)*2),30),255); //Clamp between 30 and 255, median 160
  }
  //Spotlights
  switch (spotlightPattern) {
    case 0: //off
      for (int i = 0; i < WIDTH * HEIGHT; i++) leds[NUM_SEGMENTS * LEDS_PER_LINE + i] = CRGB::Black; break;
    case 1: //Solid
      for (int i = 0; i < WIDTH * HEIGHT; i++) leds[spotlightToLedIndex(i)] = spotlights[i]; //cant call solidSpotlights since they each have their own color value
      applySpotlightBrightness();
      break;
    case 2: //rainbow"
      //This rainbow hue color is taken from the middle of the top/left segment, rather than the average/median between the 4 surronding segments.
      //If you want to change this to be the average/median of the 4 surrounding segments, get rid of the '/2' in the fill_rainbow portion: "(uint8_t)(rainbowRate*LEDS_PER_LINE)/2"
      for (int i = 0; i < WIDTH; i++) {
        int counter = 0;
        const int maxSegment = min(HEIGHT, i + 1); //amount of segments to light up in the diagonal
        for (int j = i; j < WIDTH * HEIGHT; j += (WIDTH - 1)) {
          fill_rainbow(&leds[spotlightToLedIndex(j)], 1, (uint8_t)(rainbowRate * LEDS_PER_LINE) / 2 + i * rainbowRate * LEDS_PER_LINE + hueOffset, 0);
        }
      }
      //That last spotlight(s)
      for (int i = 1; i < HEIGHT; i++) {
        for (int j = (i + 1) * WIDTH - 1; j < WIDTH * HEIGHT; j += WIDTH - 1) {
          fill_rainbow(&leds[spotlightToLedIndex(j)], 1, (uint8_t)(rainbowRate * LEDS_PER_LINE) / 2 + WIDTH * rainbowRate * LEDS_PER_LINE + hueOffset, 0);
        }
      }
      applySpotlightBrightness();
      break;
    case 3: //gradient
      gradientSpotlights(spotlights[0], spotlights[1]);
      applySpotlightBrightness();
      break;
    case 4: //rain
      //fuck me
      break;
    case 5: //sparkle
      sparkle(10 , spotlights[0] , NUM_SEGMENTS * LEDS_PER_LINE , WIDTH * HEIGHT, 255 - spotlightBrightness); //10 chance seems fine. Note this isnt 10% or 10/255% chance. See sparkle() for how the chance works
      dimSpotlights(max(spotlightBrightness / 8, 2));
      break;
  }
  //Background
  switch (backgroundPattern) {
    case 0: //off
      for (int i = 0; i < NUM_SEGMENTS * LEDS_PER_LINE; i++) leds[i] = CRGB::Black; break;
    case 1: //solid
      for (int i = 0; i < NUM_SEGMENTS * LEDS_PER_LINE; i++) leds[i] = bg; //I have a solidSegments() function, but this seems faster
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
      //fuck me
      break;
    case 5: //sparkle
      sparkle(100 , bg , 0 , NUM_SEGMENTS * LEDS_PER_LINE, 255 - backgroundBrightness); //100 chance seems fine. Note this isnt 100% or 100/255% chance. See sparkle() for how the chance works
      dimSegments(max(backgroundBrightness / 10, 2));
      break;
  }
  //Foreground
  switch (foregroundPattern) {
    case 0: break;//do nothing. Just here to acknowledge this option exists
    case 1: //solid
      if (clockRefreshTimer == FRAMES_PER_SECOND * 3) { //every 3 seconds
        updateTime();
        clockRefreshTimer = 0;
      }
      render_clock_to_display(getHour12(), getMinute(), 255 - segmentBrightness);
      clockRefreshTimer++;
      break;
    case 2: //rainbow
      if (clockRefreshTimer == FRAMES_PER_SECOND * 3) { //every 3 seconds
        updateTime();
        clockRefreshTimer = 0;
      }
      render_clock_to_display_rainbow(getHour12(), getMinute(), 255 - segmentBrightness);
      clockRefreshTimer++;
      break;
    case 3: //gradient
      if (clockRefreshTimer == FRAMES_PER_SECOND * 3) { //every 3 seconds
        updateTime();
        clockRefreshTimer = 0;
      }
      render_clock_to_display_gradient(getHour12(), getMinute(), 255 - segmentBrightness);
      clockRefreshTimer++;
      break;
  }
  hueOffset += rainbowRate;
  #ifdef BRIGHTNESS_COMPENSATION
  for(int i=0;i<sizeof(segmentBrightnessCompensation)/sizeof(segmentBrightnessCompensation[0]);i++)
    dimSegment(i,segmentBrightnessCompensation[i]);
  #endif
  FastLED.show();
}

//************************************************//
//              Display Functions                 //
//************************************************//

void clearDisplay() {               fill_solid(leds, NUM_LEDS, CRGB::Black); }
void applySegmentBrightness() {     fadeToBlackBy(leds                                        , NUM_SEGMENTS * LEDS_PER_LINE     , 255 - segmentBrightness  );}
void applySpotlightBrightness() {   fadeToBlackBy(&leds[NUM_SEGMENTS * LEDS_PER_LINE]         , WIDTH * HEIGHT                   , 255 - spotlightBrightness);}
void dimSegments(byte val) {        fadeToBlackBy(leds                                        , NUM_SEGMENTS * LEDS_PER_LINE     , val);}
void dimSpotlights(byte val) {      fadeToBlackBy(&leds[NUM_SEGMENTS * LEDS_PER_LINE]         , WIDTH * HEIGHT                   , val);}
void dimLed(int index, byte val) {  fadeToBlackBy(&leds[index]                                , 1                                , val);}
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
    if (light_tens_h & 0b01000000 >> i && h%10 == 1) { //use bitmask to see if the segment is supposed to be on for that digit
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
    if (light_tens_h & 0b01000000 >> i && h == 1) { //use bitmask to see if the segment is supposed to be on for that digit
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
    if (light_tens_h & 0b01000000 >> i && h == 1) { //use bitmask to see if the segment is supposed to be on for that digit
      gradientSegment(h_ten[i], h_ten_color, h_one_color);
      dimSegment(h_ten[i], dim);
    }
    //Change hours ones LEDS
    if (light_ones_h & 0b01000000 >> i) {
      gradientSegment(h_one[i], h_ten_color, h_one_color);
      dimSegment(h_one[i], dim);
    }
    //Change minutes tens LEDS
    if (light_tens_m & 0b01000000 >> i) {
      gradientSegment(m_ten[i], h_ten_color, h_one_color);
      dimSegment(m_ten[i], dim);
    }
    //Change minutes ones LEDS
    if (light_ones_m & 0b01000000 >> i) {
      gradientSegment(m_one[i], h_ten_color, h_one_color);
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
  for (int i = 0; i < NUM_SEGMENTS; i++)
    setSegmentColor(i, color);
}
void solidSpotlights(CRGB color) {
  for (int i = 0; i < WIDTH * HEIGHT; i++)
    leds[LEDS_PER_LINE * NUM_SEGMENTS + i ] = color;
}

void gradientSegment(int segment, CRGB color1, CRGB color2) {
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
      leds[segmentStruct.start - i] = color;
    } else {
      leds[segmentStruct.start + i] = color;
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
    leds[spotlightToLedIndex(i)] = color;
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
      leds[ledStart + randNum] = color;
      chance -= randomVal; //decrement counter so we could spawn more than 1 per iteration
      dimLed(ledStart + randNum, dim); //Apply initial brightness/dim LED when spawned
    } else break;
  }
}

void rain(byte chance, CRGB color) {
  /*
    chance/128 to spawn a raindrop
    pick random led:
    either from the WIDTH segments or:
    the top-most LED from the next WIDTH+1 segments
    for each segment
    for each led in segment
      if led = color
        drop down 1:
        if horizontal, find nearest spotlight and add 255/LEDS_PER_LINE
        if vertical, reference sign from wiring signal, then set color to below.
          if at bottom, set top LED of segment below to color
      subtract 10 from RGB
    for each spotlight
    subtract 30 from RGB using -=

  */
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
  int wiringSegmentIndex = -1;
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
  //Assume zig-zag wiring
  if ((index / WIDTH) % 2 == 0) { //forward direction
    //        spotlight offset         +   vertical offset   +  index
    return (NUM_SEGMENTS * LEDS_PER_LINE + (index / WIDTH) * WIDTH + index);
  } else { //reverse direction
    //        spotlight offset         +   vertical offset   +   reverse index
    return (NUM_SEGMENTS * LEDS_PER_LINE + (index / WIDTH) * WIDTH + WIDTH - index % WIDTH - 1);
  }
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
  lastUpdate = 0;
  updateSettings = false;
  storeEEPROM();
}

//************************************************//
//             WebServer Functions                //
//************************************************//
//Runs when site is loaded
String parseSettings() {
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
  if(foregroundPattern > sizeof(effectNames)/sizeof(effectNames[0]) || foregroundPattern < 0){
    Serial.println("Critical error: Foreground pattern number invalid: " + String(foregroundPattern));
    settings += String(effectNames[1]) + '|'; //default case
  }else settings += String(effectNames[foregroundPattern]) + '|';
  if(backgroundPattern > sizeof(effectNames)/sizeof(effectNames[0]) || backgroundPattern < 0){
    Serial.println("Critical error: Background pattern number invalid: " + String(backgroundPattern));
    settings += String(effectNames[1]) + '|'; //default case
  }else settings += String(effectNames[backgroundPattern]) + '|';
  if(spotlightPattern > sizeof(effectNames)/sizeof(effectNames[0]) || spotlightPattern < 0){
    Serial.println("Critical error: Spotlightground pattern number invalid: " + String(spotlightPattern));
    settings += String(effectNames[1]) + '|'; //default case
  }else settings += String(effectNames[spotlightPattern]);
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
}
void deleteSettings(){EEPROM.begin(512); EEPROM.write(0,0); EEPROM.commit();}

//************************************************//
//              Initialize variables              //
//************************************************//
void defaultSettings(){
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
  foregroundPattern = (byte) EEPROM.read(addr++);  
  //background pattern
  backgroundPattern = EEPROM.read(addr++);  
  //spotlight patterns
  spotlightPattern = EEPROM.read(addr++);
  //rainbow speed
  rainbowRate = EEPROM.read(addr++);
}

void storeEEPROM(){
  Serial.println("Storing to EEPROM:\n" + parseSettings());
  EEPROM.begin(512);
  int addr = 3;
  bool madeChanges = false;
  if(lightingChanges.power)                     {EEPROM.write(addr,power);                                                                                                  madeChanges = true;}     addr++;
  if(lightingChanges.foregroundTransparency)    {EEPROM.write(addr,foregroundTransparency);                                                                                 madeChanges = true;}     addr++;
  if(lightingChanges.autobrightness)            {EEPROM.write(addr,autobrightness);                                                                                         madeChanges = true;}     addr++;
  if(lightingChanges.segmentBrightness)         {EEPROM.write(addr,segmentBrightness);                                                                                      madeChanges = true;}     addr++;
  if(lightingChanges.spotlightBrightness)       {EEPROM.write(addr,spotlightBrightness);                                                                                    madeChanges = true;}     addr++;
  if(lightingChanges.backgroundBrightness)      {EEPROM.write(addr,backgroundBrightness);                                                                                   madeChanges = true;}     addr++;
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
  EEPROM.write(0,1);
  EEPROM.commit();
}

void lightingInit() {
  EEPROM.begin(512);
  if(EEPROM.read(0)==1){
    loadEEPROM();
  }
  else{
    //Load from EEPROM if it exists
    Serial.println("Creating default profile");
    defaultSettings();
    saveAllSettings();
  }
  //init verticalSegments array
  for (int i = 0; i < HEIGHT * (WIDTH + 1); i++) {
    //                                       offset from top           +   skip horizontal segments   +   segment
    //                                  0 if top, 13 if bottom         +         Width (6)            +   0/1/2/3/4/5/6
    strip seg = segmentToLedIndex( (2 * WIDTH + 1) * (i / (WIDTH + 1)) +          WIDTH               +     i % (WIDTH + 1) );
    //Place all LED indicies into the array
    if (seg.reverse) {
      for (int j = 0; j < LEDS_PER_LINE; j++) {
        //              9-j-1 if first row, 18-j-1 if bottom          0/1/2/3/4/5/6
        verticalSegments[LEDS_PER_LINE*(i / (WIDTH + 1) + 1) - j - 1][i % (WIDTH + 1)] = seg.start + j;
      }
    } else {
      for (int j = 0; j < LEDS_PER_LINE; j++) {
        //              0+j if first row, 9+j if bottom        0/1/2/3/4/5/6
        verticalSegments[ LEDS_PER_LINE*(i / (WIDTH + 1)) + j][i % (WIDTH + 1)] = seg.start + j;
      }
    }
  }
  
}
