#ifndef TIMEDEFFECTS_H
#define TIMEDEFFECTS_H
#include <FastLED.h>

void scheduleLighting();

void applyAutoSpotlightBrightness(byte effect);
void applyAutoBackgroundBrightness(byte effect);
 
void setSpotlightEffect(byte effect, CRGB c1, CRGB c2);
void setBackgroundEffect(byte effect, CRGB c1, CRGB c2);
void setSegmentEffect(byte effect, CRGB c1, CRGB c2, CRGB c3, CRGB c4);
#endif
