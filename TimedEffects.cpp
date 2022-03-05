#include "TimedEffects.h"
#include "NTPTime.h"
#include "Lighting.h"

//Brightness starting values
byte spotBrightness = 100;
byte bgBrightness   = 30;
byte segBrightness  = 60;

//LED's fade faster when these numbers are higher, and fade slower when these numbers are lower. Max value is 255 (instant fade), and lowest is 0 (no fade)
//Spotlight effect dimming rates
byte rainDimmingSpot(){    return max(spotBrightness / 8, 2);}     //reduces the brightness of spotlights by 12.5% per frame, with a minimum of 0.7% fading per frame
byte sparkleDimmingSpot(){ return max(spotBrightness / 8, 2);}     //reduces the brightness of spotlights by 12.5% per frame, with a minimum of 0.7% fading per frame
byte fireDimmingSpot(){    return max(spotBrightness / 8, 2);}     //reduces the brightness of spotlights by 12.5% per frame, with a minimum of 0.7% fading per frame

//background effect dimming rates
byte rainDimmingBg(){      return max(bgBrightness / 10, 50);}     //reduces the brightness of spotlights by 10% per frame, with a minimum of 19.6% fading per frame
byte sparkleDimmingBg(){   return max(bgBrightness / 10, 2);}      //reduces the brightness of spotlights by 10% per frame, with a minimum of 0.7% fading per frame
byte fireDimmingBg(){      return max(bgBrightness / 8, 2);}       //reduces the brightness of spotlights by 12.5% per frame, with a minimum of 0.7% fading per frame
byte loadingDimming(){     return 50;} //20% fading per frame

//Clock segment brightness handled in the code



void scheduleLighting(){
  //Example block:
  //        if current time        is later than   00:00 (12am)   and           current time           is before     06:00 (6am)   (dont go past 24:00. Ex: 23:00 -> 02:00 should be split to 23:00->24:00 and 00:00 -> 02:00)
  if(timeToMinutes(getHour24(),getMinute()) > timeToMinutes(0,00) && timeToMinutes(getHour24(),getMinute()) > timeToMinutes(6,00)){
    //----------------------------------------------------------------------------------------------------------------------------//
    //                          Apply Spotlight effects                                                                           //
    //----------------------------------------------------------------------------------------------------------------------------//
    //Effect values for spotlights:
    //0: Solid (1 color throughout. Set to CRGB(0,0,0) for off)
    //1: Addressable Solid (can set each led to a single color)
    //2: rainbow
    //3: gradient
    //4: rain
    //5: sparkle
    //6: fire
    const byte spotEffect = 1;
    
    // You can go to https://www.w3schools.com/colors/colors_picker.asp and find the rgb(###,###,###) value and plug them in here
    //Color effects will only be applied if the effects need them (Ex: rainbow wont need any colour inputs so it wont be applied)
    CRGB slcolor1 = CRGB(0,0,0); //red, green, blue. Range is from 0 (off) to 255 (on). 
    CRGB slcolor2 = CRGB(0,0,0); //only really needed for the gradient
    // If you chose to individually set each spotlight, set 'effect' to 1 and put the following lines for each spotlight
    // setSpotlightColor(0, CRGB(0,0,0)); //Sets an individual spotlight color. See Config.h to see the index number for a spotlight  
    // setSpotlightColor(1, CRGB(0,0,0)); 
    // setSpotlightColor(2, CRGB(0,0,0)); 
    // setSpotlightColor(3, CRGB(0,0,0)); 
    // setSpotlightColor(4, CRGB(0,0,0)); 
    // ....
    // Set your own brightness
    spotBrightness = 100;
    //----------------------------------------------------------------------------------------------------------------------------//
    //----------------------------------------------------------------------------------------------------------------------------//
    //                            Apply Background effects                                                                        //
    //----------------------------------------------------------------------------------------------------------------------------//
    //Effect values for background:
    //0: Off
    //1: Solid
    //2: rainbow
    //3: gradient
    //4: rain
    //5: sparkle
    //6: fire
    //255: loading loop
    const byte bgEffect = 2;
    CRGB bgcolor1 = CRGB(0,0,0);
    CRGB bgcolor2 = CRGB(0,0,0);
    bgBrightness = 30;
    //----------------------------------------------------------------------------------------------------------------------------//
    //----------------------------------------------------------------------------------------------------------------------------//
    //                            Apply Clock effects                                                                             //
    //----------------------------------------------------------------------------------------------------------------------------//
    //Effect values for clock:
    //0: Off
    //1: solid
    //2: rainbow
    //3: gradient
    const byte segEffect = 1;
    CRGB segcolor1 = CRGB(0,0,0); //hours tens-digit if solid. For gradient, this is the top left color
    CRGB segcolor2 = CRGB(0,0,0); //hours ones-digit if solid. For gradient, this is the bottom right color
    CRGB segcolor3 = CRGB(0,0,0); //minutes tens-digit if solid
    CRGB segcolor4 = CRGB(0,0,0); //minutes ones-digit if solid
    segBrightness = 60;
    //----------------------------------------------------------------------------------------------------------------------------//


    //---------------------------------------------------------------------//
    //Shouldn't need to touch these lines unless you know what you're doing//
    //---------------------------------------------------------------------//
    setSpotlightBrightness(spotBrightness); //0-255. 0 is off, 255 is full brightness
    setBackgroundBrightness(bgBrightness); //0-255. 0 is off, 255 is full brightness
    setSegmentBrightness(segBrightness); //0-255. 0 is off, 255 is full brightness
    setSpotlightEffect(spotEffect,slcolor1,slcolor2);
    applyAutoSpotlightBrightness(spotEffect); 
    setSpotlightEffect(bgEffect,bgcolor1,bgcolor2);
    applyAutoSpotlightBrightness(bgEffect);  
    setSegmentEffect(segEffect,segcolor1,segcolor2,segcolor3,segcolor4);
    //---------------------------------------------------------------------//
  }
  //Add other timed effects below here










  //------------------------------------//
  else{ //If a timeframe has not been specified, fall back to this setup
    const byte spotEffect = 2; //rainbow
    CRGB slcolor1 = CRGB(0,0,0); //Wont affect the rainbow, just here for the function parameters
    CRGB slcolor2 = CRGB(0,0,0); 
    spotBrightness = 100;
    const byte bgEffect = 2;
    CRGB bgcolor1 = CRGB(0,0,0); //Wont affect the background
    CRGB bgcolor2 = CRGB(0,0,0);
    bgBrightness = 30;
    const byte segEffect = 1;
    CRGB segcolor1 = CRGB(255,255,255); //White
    CRGB segcolor2 = CRGB(255,255,255); //White
    CRGB segcolor3 = CRGB(0,255,255); //Cyan
    CRGB segcolor4 = CRGB(0,255,255); //Cyan
    segBrightness = 60;
    setSpotlightBrightness(spotBrightness); //0-255. 0 is off, 255 is full brightness
    setBackgroundBrightness(bgBrightness); //0-255. 0 is off, 255 is full brightness
    setSegmentBrightness(segBrightness); //0-255. 0 is off, 255 is full brightness
    setSpotlightEffect(spotEffect,slcolor1,slcolor2);
    applyAutoSpotlightBrightness(spotEffect); 
    setSpotlightEffect(bgEffect,bgcolor1,bgcolor2);
    applyAutoSpotlightBrightness(bgEffect);  
    setSegmentEffect(segEffect,segcolor1,segcolor2,segcolor3,segcolor4);
    //---------------------------------------------------------------------//
  }
}

void applyAutoSpotlightBrightness(byte effect){
  switch(effect){
    case 0: case 1: case 2: case 3:
      setSpotlightBrightness(spotBrightness);
      applySpotlightBrightness(); break;
    case 4: 
      dimSpotlights(rainDimmingSpot()); break;
    case 5: 
      dimSpotlights(sparkleDimmingSpot()); break;
    case 6:
      dimSpotlights(fireDimmingSpot()); break;
  }
}
void applyAutoBackgroundBrightness(byte effect){
  switch(effect){
    case 0: case 1: case 2: case 3:
      dimSegments(255 - bgBrightness); break;
    case 4:
      dimSegments(rainDimmingBg()); break;
    case 5:
      dimSegments(sparkleDimmingBg()); break;
    case 6:
      dimSegments(fireDimmingBg()); break;
    case 255:
      dimSpotlights(loadingDimming()); break;
  }
}

void setSpotlightEffect(byte effect, CRGB c1, CRGB c2){
  switch (spotlightPattern) {
    case 0: solidSpotlights(c1); break;
    case 1: solidUniqueSpotlights(); break;
    case 2: rainbowSpotlights(); break;
    case 3: gradientSpotlights(c1, c2); break;
    case 4: rain(30, CRGB::Black, c1); break; //note that 30 is the threshold for spawning. A random number (0-128) is subtracted from that number per frame until it can no longer be subtracted to stay >0.
    case 5: sparkle(10 , c1 , NUM_SEGMENTS * LEDS_PER_LINE , WIDTH * HEIGHT, 255 - spotBrightness); break; //note that 10 is the threshold for spawning. A random number (0-128) is subtracted from that number per frame until it can no longer be subtracted to stay >0.
    case 6: fire() ;break;
  }
}
void setBackgroundEffect(byte effect, CRGB c1, CRGB c2){
  switch (effect) {
      case 0: solidSegments(CRGB::Black); break;
      case 1: solidSegments(c1); break;
      case 2: for (int i = 0; i < NUM_SEGMENTS; i++) rainbowSegment(i, segmentLightingOffset(i)*LEDS_PER_LINE * rainbowRate, rainbowRate); break;
      case 3: for (int i = 0; i < NUM_SEGMENTS; i++) gradientSegment(i, c1, c2); break;
      case 4: rain(15, c1, CRGB::Black); break;
      case 5: sparkle(100 , c1 , 0 , NUM_SEGMENTS * LEDS_PER_LINE, 255 - bgBrightness); break; //100 chance seems fine. Note this isnt 100% or 100/255% chance. See sparkle() for how the chance works
      case 6: if(spotlightPattern != 6) fire(); break;
      case 255: loadingEffect(c1); break;
    }
}
void setSegmentEffect(byte effect, CRGB c1, CRGB c2, CRGB c3, CRGB c4){
  h_ten_color = c1;
  h_one_color = c2;
  m_ten_color = c3;
  m_one_color = c4;
  switch (effect) {
      case 0: break;//do nothing. Just here to acknowledge this option exists
      case 1: //solid
        if (clockRefreshTimer == FRAMES_PER_SECOND * 3) { updateTime();clockRefreshTimer = 0;}
        #ifdef _12_HR_CLOCK
        render_clock_to_display(getHour12(), getMinute(), 255 - segBrightness);
        #elif defined(_24_HR_CLOCK)
        render_clock_to_display(getHour24(), getMinute(), 255 - segBrightness);
        #endif
        clockRefreshTimer++;
        break;
      case 2: //rainbow
        if (clockRefreshTimer == FRAMES_PER_SECOND * 3) { updateTime();clockRefreshTimer = 0;}
        #ifdef _12_HR_CLOCK
        render_clock_to_display_rainbow(getHour12(), getMinute(), 255 - segBrightness);
        #elif defined(_24_HR_CLOCK)
        render_clock_to_display_rainbow(getHour24(), getMinute(), 255 - segBrightness);
        #endif
        clockRefreshTimer++;
        break;
      case 3: //gradient
        if (clockRefreshTimer == FRAMES_PER_SECOND * 3) { updateTime();clockRefreshTimer = 0;}
        #ifdef _12_HR_CLOCK
        render_clock_to_display_gradient(getHour12(), getMinute(), 255 - segBrightness);
        #elif defined(_24_HR_CLOCK)
        render_clock_to_display_gradient(getHour24(), getMinute(), 255 - segBrightness);
        #endif
        clockRefreshTimer++;
        break;
    }
}
