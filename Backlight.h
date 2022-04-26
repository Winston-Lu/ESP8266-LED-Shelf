#ifndef BACKLIGHT_H
#define BACKLIGHT_H
#include <FastLED.h>
void initBacklight();
void showBacklight();
void pushEffectTable();
int getBacklightEffectID();
void setBacklightEffectID(int val);
void setBacklightColor(byte pos, CRGB color);
void setBacklightBrightness(byte val);
void saveBLEEPROM();
#endif
