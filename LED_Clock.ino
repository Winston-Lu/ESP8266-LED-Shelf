#include <LittleFS.h> //for filesystem
#include "Config.h"
#include "NTPTime.h"
#include "Lighting.h"
#include "WebServer.h" 

byte FRAMES_PER_SECOND = 30; //will be overwritten later on by EEPROM or default settings
unsigned long frameStart; //For fps counter

void setup(){
  Serial.begin(115200);
  Serial.println("\n\n\n\n\n"); //get rid of the jiberish from boot


  Serial.println("Config Settings");
  Serial.print("Number of segments: "); Serial.println(NUM_SEGMENTS);
  Serial.print("LED's on segment pin: "); Serial.println(NUM_LEDS);
  #ifdef SPOTLIGHTPIN
    Serial.print("LED's on spotlight pin: "); Serial.println(WIDTH*HEIGHT);
    Serial.print("Total LED's:"); Serial.println(NUM_LEDS+WIDTH*HEIGHT);
  #endif
  Serial.print("Seperate pin for spotlights: ");
  #ifdef SPOTLIGHTPIN
    Serial.print("Pin "); Serial.println(SPOTLIGHTPIN);
  #elif 
    Serial.println("Not defined");
  #endif
  Serial.print("Clock mode: ");
  #ifdef _12_HR_CLOCK
    Serial.println("12 Hour");
  #elif 
    Serial.println("24 Hour");
  #endif
  #ifdef SACRIFICELED
    Serial.println("Sacrifice LED Enabled");
  #endif
  Serial.println("\n");

  pinMode(LIGHT_SENSOR,INPUT);
  random16_add_entropy((uint16_t)random16());
  for(int i=0;i<10;i++) random16_add_entropy(random(65535));
  
  //FastLED Setup
  fastLEDInit();
  FastLED.setDither(false);
  FastLED.setBrightness(255);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  solidSegments(CRGB::Black);
  solidSpotlights(CRGB::Black);
  
  Serial.println("Initializing data structures for lighting effects");
  lightingInit();

  //Website filesystem setup
  LittleFS.begin();
  Serial.println("LittleFS contents:");
  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    String fileName = dir.fileName();
    size_t fileSize = dir.fileSize();
    Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
  }
  Serial.println("\n");
  //WiFi Setup
  Serial.println("Setting up Wi-Fi");
  setupWiFi();
  Serial.println("Showing lights");
  showLightingEffects();
  Serial.println("Initializing real-time clock...");
  initClock();
  Serial.println("Getting webserver started...");
  setupServer();
}

uint16_t counter = 0;
void loop() {
  if (counter==FRAMES_PER_SECOND*5) frameStart = micros();
  
  updateServer();
  showLightingEffects();
  
  if(counter>=FRAMES_PER_SECOND*5){
    unsigned long microsecondsPerFrame = micros()-frameStart;
    char buff[60];
    sprintf(buff, "Maximum FPS: %.1f     Milliseconds per frame: %.2f",1000000.0/microsecondsPerFrame,microsecondsPerFrame/1000.0);
    Serial.println(buff);
    counter = 0;
  }
  counter++;
  
  lastUpdate++;
  if(lastUpdate >= EEPROM_UPDATE_DELAY*FRAMES_PER_SECOND && updateSettings){
    lastUpdate = 0;
    updateSettings = false;
    storeEEPROM();
  }
  //if we have a sacrifice LED, shift all LED's down by 1 and set first to black (probably wont end up being black)
  #ifdef SACRIFICELED
  shiftLedsByOne();
  #endif
  // insert a delay to maintain framerate. Also does FastLED.show()
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}
