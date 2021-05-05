#include <LittleFS.h> //for filesystem
#include "Config.h"
#include "NTPTime.h"
#include "Lighting.h"
#include "WebServer.h" 

byte FRAMES_PER_SECOND = 30; //will be overwritten later on by EEPROM or default settings

void setup(){
  Serial.begin(115200);
  random16_add_entropy((uint16_t)random16());
  pinMode(LIGHT_SENSOR,INPUT);
  Serial.println("\n\n\n\n\n"); //get rid of the jiberish from boot
  
  //FastLED Setup
  FastLED.addLeds<LED_TYPE, DATAPIN, COLOR_ORDER>(leds, NUM_LEDS);         
  FastLED.setDither(false);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(255);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MILLI_AMPS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  
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
  Serial.printf("\n");
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

void loop() {
  updateServer();
  
  lastUpdate++;
  if(lastUpdate >= EEPROM_UPDATE_DELAY*FRAMES_PER_SECOND && updateSettings){
    lastUpdate = 0;
    updateSettings = false;
    storeEEPROM();
  }

  if (power == 0) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    FastLED.show();
    delay(1000 / FRAMES_PER_SECOND);
    return;
  }
  showLightingEffects();

  // insert a delay to keep the framerate modest
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}
