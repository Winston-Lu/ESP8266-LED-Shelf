#ifndef LIGHTING_H
#define LIGHTING_H

#include <FastLED.h>
#include "Config.h"

#ifdef SPOTLIGHTPIN
const int PROGMEM NUM_LEDS = LEDS_PER_LINE * (2*WIDTH*HEIGHT + WIDTH + HEIGHT);
#else
const int PROGMEM NUM_LEDS = LEDS_PER_LINE * (2*WIDTH*HEIGHT + WIDTH + HEIGHT) + WIDTH*HEIGHT;
#endif

const int PROGMEM NUM_SEGMENTS = 2*WIDTH*HEIGHT + WIDTH + HEIGHT;

//Love me some global variables
extern int rainbowRate;
extern int hueOffset;
extern uint32_t lastUpdate;
extern bool updateSettings;

struct strip {
  int start;
  bool reverse;
};
struct changelist{
  bool power = false;
  bool foregroundTransparency = false;
  bool autobrightness = false;
  bool segmentBrightness = false;
  bool spotlightBrightness = false;
  bool backgroundBrightness = false;
  bool h_ten_color = false;
  bool h_one_color = false;
  bool m_ten_color = false;
  bool m_one_color = false;
  bool bg = false;
  bool bg2 = false;
  bool spotlights[WIDTH*HEIGHT];
  bool foregroundPattern = false;
  bool backgroundPattern = false;
  bool spotlightPattern = false;
  bool rainbowRate = false;
  bool fps = false;
  bool hyphenLength = false;
  bool hyphenColor = false;
};

extern changelist lightingChanges;

//Web-Server global variables
extern int power;
extern boolean autobrightness;
extern boolean autoEffect;
extern CRGB h_ten_color;
extern CRGB h_one_color;
extern CRGB m_ten_color;
extern CRGB m_one_color;
extern CRGB bg;
extern CRGB bg2;
extern byte foregroundPattern;
extern byte backgroundPattern;
extern byte spotlightPattern;
extern bool updateSettings;
extern byte hyphenLength;
extern CRGB hyphenColor;
extern uint32_t clockRefreshTimer; 

//Display Functions
void clearDisplay();
void applySegmentBrightness();
void applySpotlightBrightness();
void dimSegments(byte val);
void dimSpotlights(byte val);
void dimLed(int index, byte val);
void dimSegment(int segment, byte val);

//Effect Handler
void showLightingEffects();

//Clock
void render_clock_to_display(int h, int m);
void render_clock_to_display(int h, int m, byte dim);
void render_clock_to_display_rainbow(int h, int m);
void render_clock_to_display_rainbow(int h, int m, byte dim);
void render_clock_to_display_gradient(int h, int m);
void render_clock_to_display_gradient(int h, int m, byte dim);

//Color Effects
void setSegmentColor(int segment, CRGB color);
void addSegmentColor(int segment, CRGB color, byte transparency);
void setSpotlightColor(int index, CRGB color);
void solidSegments(CRGB color);
void solidSpotlights(CRGB color);
void solidUniqueSpotlights();
void gradientSegment(int segment, CRGB color1, CRGB color2);
void gradientSegment(int segment, CRGB color1, CRGB color2, byte transparency);
void gradientSpotlights(CRGB color1, CRGB color2);
void rainbow(int rate);
void rainbowSegment(int segment, uint8_t offset, uint8_t rate);
void rainbowSegment(int segment, uint8_t offset, uint8_t rate, byte transparency);
void rainbowSpotlights();
void sparkle(int chance, int ledStart, int len);
void sparkle(int chance, CRGB color, int ledStart, int len);
void sparkle(int chance, CRGB color, int ledStart, int len, byte dim);
void rain(byte chance, CRGB color, CRGB spotlightColor);
void fire();
void loadingEffect(CRGB color);

//Helper Functions
byte segmentLightingOffset(int index);
strip segmentToLedIndex(int index);
int spotlightToLedIndex(int index);
int spotlightToLedIndexDedicated(int index);
CRGB dimColor(CRGB color,byte amount);
uint8_t sevenSegment(int num);
void storeEEPROM();
void storeUtcOffset(double value);
void resetEEPROM();
void saveAllSettings();
double getUtcOffset();
void clearLightingCache();
String getCurrentSettings(String seperator);
void shiftLedsByOne();
int timeToMinutes(int h, int m);

//Web-Server Functions
String parseSettings();
void setSegmentBrightness(byte brightness);
void setSpotlightBrightness(byte brightness);
void setBackgroundBrightness(byte brightness);
void setForegroundTransparency(byte trans);
void setSpotlight1(CRGB color);
void setSpotlight2(CRGB color);
String crgbToCss(CRGB val);
String byteToHexString(byte num);
String numToHex(byte num);
void deleteSettings();

//Initialization function
void fastLEDInit();
void defaultSettings();
void lightingInit();
void showLights();

//Index of each segment for each digit (abstracted index, not wiring index)
#ifdef _12_HR_CLOCK
  const uint32_t PROGMEM tens_h[7] = {0,WIDTH  ,0    ,0      ,3*WIDTH  ,0      ,0}; 
  const uint32_t PROGMEM ones_h[7] = {1,WIDTH+1,WIDTH+2,2*WIDTH+1,3*WIDTH+1,3*WIDTH+2,4*WIDTH+1};
  const uint32_t PROGMEM tens_m[7] = {3,WIDTH+3,WIDTH+4,2*WIDTH+3,3*WIDTH+3,3*WIDTH+4,4*WIDTH+3};
  const uint32_t PROGMEM ones_m[7] = {5,WIDTH+5,WIDTH+6,2*WIDTH+5,3*WIDTH+5,3*WIDTH+6,4*WIDTH+5};
#elif defined(_24_HR_CLOCK)
  const uint32_t PROGMEM tens_h[7] = {0,WIDTH  ,WIDTH+1,2*WIDTH  ,3*WIDTH  ,3*WIDTH+1,4*WIDTH  };
  const uint32_t PROGMEM ones_h[7] = {2,WIDTH+2,WIDTH+3,2*WIDTH+2,3*WIDTH+2,3*WIDTH+3,4*WIDTH+2};
  const uint32_t PROGMEM tens_m[7] = {4,WIDTH+4,WIDTH+5,2*WIDTH+4,3*WIDTH+4,3*WIDTH+5,4*WIDTH+4};
  const uint32_t PROGMEM ones_m[7] = {6,WIDTH+6,WIDTH+7,2*WIDTH+6,3*WIDTH+6,3*WIDTH+7,4*WIDTH+6};
#endif
#endif
