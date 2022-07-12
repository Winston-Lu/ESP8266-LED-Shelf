#ifdef BACKLIGHT_LED

#include <FastLED.h>
#include <EEPROM.h> 
#include "Config.h"
#include "Backlight.h"
#include "Lighting.h"

#define BACKLIGHT_NUM_LEDS 2*(BACKLIGHT_WIDTH + BACKLIGHT_HEIGHT)

CRGB effectTable[ BACKLIGHT_WIDTH + BACKLIGHT_HEIGHT ]; //for pre-computing values to lookup and reduce computations
CRGB backlight_leds[ BACKLIGHT_NUM_LEDS ];
int ledIndex[ BACKLIGHT_NUM_LEDS ];
int effectID = 0;
uint64_t lastSave = 0;
byte backlightBrightness = 255;

CRGB backlight1=CRGB::Black, 
     backlight2=CRGB::Black;

//Set indicies to a standard order
void initBacklight(){
  #if defined(BACKLIGHT_TOP_LEFT) && defined(BACKLIGHT_CLOCKWISE) 
    for(int i=0;i<BACKLIGHT_NUM_LEDS;i++)
      ledIndex[i] = i;
      
  #elif defined(BACKLIGHT_TOP_RIGHT) && defined(BACKLIGHT_CLOCKWISE) 
    for(int i=0;i<BACKLIGHT_NUM_LEDS;i++)
      ledIndex[i] = (i+BACKLIGHT_WIDTH) % BACKLIGHT_NUM_LEDS ;
    
  #elif defined(BACKLIGHT_TOP_LEFT) && defined(BACKLIGHT_COUNTERCLOCKWISE) 
    for(int i=0;i<BACKLIGHT_NUM_LEDS;i++)
      ledIndex[i] = i-BACKLIGHT_NUM_LEDS-1 ;

  #elif defined(BACKLIGHT_TOP_RIGHT) && defined(BACKLIGHT_COUNTERCLOCKWISE) 
    for(int i=0;i<BACKLIGHT_NUM_LEDS;i++)
        ledIndex[i] = (BACKLIGHT_WIDTH-1-i) % BACKLIGHT_NUM_LEDS;

  #elif defined(BACKLIGHT_LED)
    #error "Backlight enabled but settings are invalid. Please check if you set options to BACKLIGHT_TOP_RIGHT or BACKLIGHT_TOP_LEFT, and BACKLIGHT_CLOCKWISE or BACKLIGHT_COUNTERCLOCKWISE"
  #endif
  FastLED.addLeds<LED_TYPE, BACKLIGHT_PIN, COLOR_ORDER>(backlight_leds, BACKLIGHT_NUM_LEDS).setCorrection(TypicalLEDStrip); 
  EEPROM.begin(512);
  byte addr = 36 + WIDTH*HEIGHT*3; //Offset
  effectID = EEPROM.read(addr++);
  backlightBrightness = EEPROM.read(addr++);
  backlight1 = CRGB(EEPROM.read(addr),EEPROM.read(addr+1),EEPROM.read(addr+2));  addr+=3;
  backlight2 = CRGB(EEPROM.read(addr),EEPROM.read(addr+1),EEPROM.read(addr+2));  addr+=3;
  
}
void setBacklightEffectID(int val){effectID = val;}
int getBacklightEffectID(){return effectID;}
void setBacklightBrightness(byte val){backlightBrightness = val;}
void setBacklightColor(byte pos, CRGB color){
  if      (pos == 0) backlight1 = color;
  else if (pos == 1) backlight2 = color;
}

void saveBLEEPROM(){
  byte addr = 36 + WIDTH*HEIGHT*3; //Offset
  EEPROM.begin(512);
  EEPROM.write(addr  ,effectID); 
  EEPROM.write(addr+1,backlightBrightness); 
  EEPROM.write(addr+2,backlight1.red);    
  EEPROM.write(addr+3,backlight1.green);   
  EEPROM.write(addr+4,backlight1.blue); 
  EEPROM.write(addr+5,backlight2.red);    
  EEPROM.write(addr+6,backlight2.green);   
  EEPROM.write(addr+7,backlight2.blue); 
  EEPROM.commit();
}

int loopIndex = 0;
byte sparkle_chance = 10; //increase this to spawn more sparkles

void showBacklight(){
  switch(effectID){
    case 0: //off
      for(int i=0;i<BACKLIGHT_NUM_LEDS;i++)
        backlight_leds[ledIndex[i]] = CRGB::Black;
      fadeToBlackBy(backlight_leds, BACKLIGHT_NUM_LEDS , 255 - backlightBrightness );
      break;
    case 1: //solid
      for(int i=0;i<BACKLIGHT_NUM_LEDS;i++)
        backlight_leds[ledIndex[i]] = backlight1;
      fadeToBlackBy(backlight_leds, BACKLIGHT_NUM_LEDS , 255 - backlightBrightness );
      break;
    case 2: //rainbow
      fill_rainbow(effectTable,BACKLIGHT_WIDTH + BACKLIGHT_HEIGHT,hueOffset,rainbowRate);
      pushEffectTable();
      fadeToBlackBy(backlight_leds, BACKLIGHT_NUM_LEDS , 255 - backlightBrightness );
      break;
    case 3: //gradient
      fill_gradient_RGB(effectTable,BACKLIGHT_WIDTH + BACKLIGHT_HEIGHT,backlight1,backlight2);
      pushEffectTable();
      fadeToBlackBy(backlight_leds, BACKLIGHT_NUM_LEDS , 255 - backlightBrightness );
      break;
    case 4: //rain TODO
      break;
    case 5: //sparkle
      while (1) {
        byte randomVal = random8(128);
        if (sparkle_chance > randomVal) {
          uint16_t randNum = random16(BACKLIGHT_NUM_LEDS); //pick random index within the specified range
          sparkle_chance -= randomVal; //decrement counter so we could spawn more than 1 per iteration
          backlight_leds[randNum] = backlight1; //apply sparkle to segments and spotlights if the value of ledStart and len permit
          fadeToBlackBy(backlight_leds+randNum, 1 , 255 - backlightBrightness ); //dim that sparkle
        } else break;
      }
      fadeToBlackBy(backlight_leds, BACKLIGHT_NUM_LEDS , 60 );
      break;
    case 6: //fire TODO
      break;
    case 255: //loop
      fadeToBlackBy(backlight_leds, BACKLIGHT_NUM_LEDS , 255 - backlightBrightness );
      backlight_leds[ledIndex[loopIndex++]] = backlight1;
      break;
    default:
      effectID=0; //in case it was invalid
  }
}

// Translates the data in effectTable to the background LED's
void pushEffectTable(){
  //horizontal
  for(int i=0;i<BACKLIGHT_WIDTH;i++){
    //top horizontal
    backlight_leds[ledIndex[i]] = effectTable[i]; 
    //bottom horizontal
    backlight_leds[ledIndex[BACKLIGHT_NUM_LEDS - BACKLIGHT_HEIGHT - 1 - i]] = effectTable[i+BACKLIGHT_HEIGHT];
  }
  //vertical
  for(int i=0;i<BACKLIGHT_WIDTH;i++){
    //right vertical
    backlight_leds[ledIndex[i+BACKLIGHT_WIDTH] ] = effectTable[i + BACKLIGHT_WIDTH]; 
    //left vertical
    backlight_leds[ledIndex[BACKLIGHT_NUM_LEDS - 1 - i]] = effectTable[i];
  }
}
#endif
