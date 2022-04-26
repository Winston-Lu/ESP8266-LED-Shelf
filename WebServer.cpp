#include <ESP8266WebServer.h>
#include <FastLED.h>
#include <LittleFS.h> //for file system
#include "WebServer.h"
#include "Lighting.h"
#include "NTPTime.h"
#include "Backlight.h"

ESP8266WebServer webServer(80);


void updateServer(){
  webServer.handleClient();
}

void setupServer(){
  //Runs when site is loaded
  webServer.on("/getsettings", HTTP_GET, []() {
    String value = parseSettings();
    Serial.println("Sending: " + value);
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "text/plain", value);
  });

  webServer.on("/power", HTTP_GET, []() {
    power = (webServer.arg("value")=="true") ? 1 : 0;
    lightingChanges.power = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });
  
  webServer.on("/transparency", HTTP_GET, []() {
    setForegroundTransparency((byte)webServer.arg("value").toInt()%256);
    lightingChanges.foregroundTransparency = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/autobrightness", HTTP_GET, []() {
    autobrightness = (webServer.arg("value")=="true")%256;
    lightingChanges.autobrightness = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/brightness", HTTP_GET, []() {
    setSegmentBrightness((byte)webServer.arg("value").toInt()%256);
    lightingChanges.segmentBrightness = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/spotlightbrightness", HTTP_GET, []() {
    setSpotlightBrightness((byte)webServer.arg("value").toInt()%256);
    lightingChanges.spotlightBrightness = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/backgroundbrightness", HTTP_GET, []() {
    setBackgroundBrightness((byte)webServer.arg("value").toInt()%256);
    lightingChanges.backgroundBrightness = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });
  
  webServer.on("/foregroundTransparency", HTTP_GET, []() {
    setForegroundTransparency((byte)webServer.arg("value").toInt()%256);
    lightingChanges.foregroundTransparency = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/bgcolor", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    color.red = webServer.arg("red").toInt()%256;
    color.green = webServer.arg("green").toInt()%256;
    color.blue = webServer.arg("blue").toInt()%256;
    bg = color;
    lightingChanges.bg = true;
    lastUpdate = 0;
    autoEffect = 0; //turn off scheduled lighting effects and toggle back on regular effects
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/bg2color", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    color.red = webServer.arg("red").toInt()%256;
    color.green = webServer.arg("green").toInt()%256;
    color.blue = webServer.arg("blue").toInt()%256;
    bg2 = color;
    lightingChanges.bg2 = true;
    lastUpdate = 0;
    autoEffect = 0; //turn off scheduled lighting effects and toggle back on regular effects
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/h1color", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    color.red = webServer.arg("red").toInt()%256;
    color.green = webServer.arg("green").toInt()%256;
    color.blue = webServer.arg("blue").toInt()%256;
    h_ten_color = color;
    lightingChanges.h_ten_color = true;
    lastUpdate = 0;
    autoEffect = 0; //turn off scheduled lighting effects and toggle back on regular effects
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/h2color", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    color.red = webServer.arg("red").toInt()%256;
    color.green = webServer.arg("green").toInt()%256;
    color.blue = webServer.arg("blue").toInt()%256;
    h_one_color = color;
    lightingChanges.h_one_color = true;
    lastUpdate = 0;
    autoEffect = 0; //turn off scheduled lighting effects and toggle back on regular effects
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  
  webServer.on("/m1color", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    color.red = webServer.arg("red").toInt()%256;
    color.green = webServer.arg("green").toInt()%256;
    color.blue = webServer.arg("blue").toInt()%256;
    m_ten_color = color;
    lightingChanges.m_ten_color = true;
    lastUpdate = 0;
    autoEffect = 0; //turn off scheduled lighting effects and toggle back on regular effects
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/m2color", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    color.red = webServer.arg("red").toInt()%256;
    color.green = webServer.arg("green").toInt()%256;
    color.blue = webServer.arg("blue").toInt()%256;
    m_one_color = color;
    lightingChanges.m_one_color = true;
    lastUpdate = 0;
    autoEffect = 0; //turn off scheduled lighting effects and toggle back on regular effects
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });
  
  webServer.on("/spotcolor", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    int index = webServer.arg("value").toInt();
    color.red = webServer.arg("red").toInt()%256;
    color.green = webServer.arg("green").toInt()%256;
    color.blue = webServer.arg("blue").toInt()%256;
    if(index >= WIDTH*HEIGHT){webServer.send(400, "text/plain", index + " not supported"); return;}
    setSpotlightColor(index-1,color);
    if(index==1) setSpotlight1(color);
    else if(index==2) setSpotlight2(color);
    
    lightingChanges.spotlights[index-1] = true;
    lastUpdate = 0;
    autoEffect = 0; //turn off scheduled lighting effects and toggle back on regular effects
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/effectfg", HTTP_GET, []() {
    String pattern = webServer.arg("value");
    if(pattern == "off") foregroundPattern = 0;
    else if(pattern == "solid") foregroundPattern = 1;
    else if(pattern == "rainbow") foregroundPattern = 2;
    else if(pattern == "gradient") foregroundPattern = 3;
    else{webServer.send(400, "text/plain", pattern + " not supported"); return;}
    lightingChanges.foregroundPattern = true;
    lastUpdate = 0;
    autoEffect = 0; //turn off scheduled lighting effects and toggle back on regular effects
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/effectbg", HTTP_GET, []() {
    String pattern = webServer.arg("value");
    if(pattern == "off") backgroundPattern = 0;
    else if(pattern == "solid") backgroundPattern = 1;
    else if(pattern == "rainbow") backgroundPattern = 2;
    else if(pattern == "gradient") backgroundPattern = 3;
    else if(pattern == "rain") backgroundPattern = 4;
    else if(pattern == "sparkle") backgroundPattern = 5;
    else if(pattern == "fire") backgroundPattern = 6;
    else if(pattern == "loop") backgroundPattern = 255;
    else{webServer.send(400, "text/plain", pattern + " not supported"); return;}
    lightingChanges.backgroundPattern = true;
    clearLightingCache();
    lastUpdate = 0;
    autoEffect = 0; //turn off scheduled lighting effects and toggle back on regular effects
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });
  
  webServer.on("/effectsl", HTTP_GET, []() {
    String pattern = webServer.arg("value");
    if(pattern == "off") spotlightPattern = 0;
    else if(pattern == "solid") spotlightPattern = 1;
    else if(pattern == "rainbow") spotlightPattern = 2;
    else if(pattern == "gradient") spotlightPattern = 3;
    else if(pattern == "rain") spotlightPattern = 4;
    else if(pattern == "sparkle") spotlightPattern = 5;
    else if(pattern == "fire") spotlightPattern = 6;
    else{webServer.send(400, "text/plain", pattern + " not supported"); return;}
    lightingChanges.spotlightPattern = true;
    clearLightingCache();
    lastUpdate = 0;
    autoEffect = 0; //turn off scheduled lighting effects and toggle back on regular effects
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });
  #ifdef BACKLIGHT
  webServer.on("/effectbl", HTTP_GET, []() {
    String pattern = webServer.arg("value");
    if(pattern == "off") setBacklightEffectID(0);
    else if(pattern == "solid") setBacklightEffectID(1);
    else if(pattern == "rainbow") setBacklightEffectID(2);
    else if(pattern == "gradient") setBacklightEffectID(3);
    else if(pattern == "rain") setBacklightEffectID(4);
    else if(pattern == "sparkle") setBacklightEffectID(5);
    else if(pattern == "fire") setBacklightEffectID(6);
    else if(pattern == "loop") setBacklightEffectID(255);
    else{webServer.send(400, "text/plain", pattern + " not supported"); return;}
    lastUpdate = 0;
    autoEffect = 0; //turn off scheduled lighting effects and toggle back on regular effects
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });
  webServer.on("/blcolor", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    color.red = webServer.arg("red").toInt()%256;
    color.green = webServer.arg("green").toInt()%256;
    color.blue = webServer.arg("blue").toInt()%256;
    setBacklightColor(0, color);
    lastUpdate = 0;
    autoEffect = 0; //turn off scheduled lighting effects and toggle back on regular effects
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });
  webServer.on("/bl2color", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    color.red = webServer.arg("red").toInt()%256;
    color.green = webServer.arg("green").toInt()%256;
    color.blue = webServer.arg("blue").toInt()%256;
    setBacklightColor(1, color);
    lastUpdate = 0;
    autoEffect = 0; //turn off scheduled lighting effects and toggle back on regular effects
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });
  webServer.on("/backlightbrightness", HTTP_GET, []() {
    setBacklightBrightness((byte)webServer.arg("value").toInt()%256);
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });
  #endif
  webServer.on("/utcoffset", HTTP_GET, []() {
    storeUtcOffset(webServer.arg("value").toDouble());
    setNewOffset();
    webServer.send(200, "text/plain", "1");
  });
   
  webServer.on("/cmd", HTTP_GET, []() {
    String command = webServer.arg("c");
    String val = webServer.arg("v");
    command.toLowerCase();
    val.toLowerCase();
    if(command=="help" || command=="?"){
      String resp = String("Available commands:<br> settings - Gets the current settings<br>" + String("") +  //Doesnt like it when I just do *char[] + *char[] here, so I need to do this
                                        "utcoffset ## - Set UTF offset in hours for clock [current offset: " + String(getOffset()) + "]<br>" +
                                        "rainbowrate ## - Set how rainbowy the rainbow is [current rate: " + String(rainbowRate) + "]<br>" +
                                        "fps ## - Sets frames per second [current rate: " + String(FRAMES_PER_SECOND) + "]<br>" +
                                        "effecttimer ## - Turns on or off scheduled time configurations. [current value: " + String(autoEffect) + "]<br>" +
                                        "hyphen ## - Sets the length of the hyphen seperator. 0 Disables hyphen. [current length: " + String(hyphenLength) + "]<br>" +
                                        "hyphencolor HEX - Sets the hyphen color (format should be RRGGBB in hex, like FFA400). [current color: " + String(crgbToCss(hyphenColor)) + "]<br>" +
                                        "reset - Reset all settings<br>" +
                                        "resetprofile - Reset lighting settings<br>" +
                                        "save - Saves all current settings<br>" +
                                        "reboot - Reboots the device after 3 seconds");
      webServer.send(200, "text/plain", resp);
    }else if(command=="settings"){
      webServer.send(200, "text/plain", getCurrentSettings("<br>"));
    }else if(command=="utcoffset"){
      utcOffset = val.toDouble();
      setNewOffset();
      updateTime();
      storeUtcOffset(utcOffset);
      webServer.send(200, "text/plain", "Set UTC offset to " + String(getUtcOffset()));
    }else if(command=="rainbowrate"){
      rainbowRate = (double)val.toInt();
      lightingChanges.rainbowRate = true;
      lastUpdate = EEPROM_UPDATE_DELAY*FRAMES_PER_SECOND;
      updateSettings = true;
      webServer.send(200, "text/plain", "Set rainbow rate to " + String(rainbowRate));
    }else if(command=="fps"){
      FRAMES_PER_SECOND = max((byte)1,(byte)val.toInt()); //Setting to 0 sets off the watchdog since it never refreshes
      lightingChanges.fps = true;
      lastUpdate = EEPROM_UPDATE_DELAY*FRAMES_PER_SECOND;
      updateSettings = true;
      webServer.send(200, "text/plain", "Set fps to " + String(FRAMES_PER_SECOND));
    }else if(command=="reset"){
      defaultSettings();
      saveAllSettings();
      webServer.send(200, "text/plain", "Reset Settings");
    }else if(command=="resetprofile"){
      deleteSettings();
      webServer.send(200, "text/plain", "Profile will reset to default on reboot");
    }else if(command=="hyphen"){
      hyphenLength = max(min((byte)val.toInt(),(byte)LEDS_PER_LINE),(byte)0);
      lightingChanges.hyphenLength = true;
      lastUpdate = EEPROM_UPDATE_DELAY*FRAMES_PER_SECOND;
      updateSettings = true;
      webServer.send(200, "text/plain", "Set hyphen length to " + String(hyphenLength));
    }else if(command=="hyphencolor"){
      Serial.println("Color: #" + val.substring(0,2) + val.substring(2,4) + val.substring(4,6));
      Serial.println("Value: " + String(hexToByte(val.substring(0,2))) + " " + String(hexToByte(val.substring(2,4))) + " " + String(hexToByte(val.substring(4,6))));
      hyphenColor = CRGB(hexToByte(val.substring(0,2)),hexToByte(val.substring(2,4)),hexToByte(val.substring(4,6)));
      lightingChanges.hyphenColor = true;
      lastUpdate = EEPROM_UPDATE_DELAY*FRAMES_PER_SECOND;
      updateSettings = true;
      webServer.send(200, "text/plain", "Set hyphen color to " + String(crgbToCss(hyphenColor)));
    }else if(command=="save"){
      saveAllSettings();
      webServer.send(200, "text/plain", "Saved Settings");
    }else if(command=="reboot" || command=="restart"){
      //Wont actually send
      //webServer.send(200, "text/plain", "Rebooting in 3 seconds...");
      
      ESP.wdtEnable(3000); // Turn on watchdog then let program hang. Im not sure if the 15ms is actually implemented in the ESP source code
      for(;;) {} //Make watchdog reset the device
    }else if(command="effecttimer"){
      autoEffect = (val.toInt() != 0); //force bool cast
      webServer.send(200, "text/plain", "Set effecttimer to " + String(autoEffect));
    }else{
      webServer.send(200, "text/plain", "Command not found");
    }
  });

  webServer.serveStatic("/", LittleFS, "/", "max-age=86400");

  webServer.begin();
  Serial.println("HTTP web server initialized");
}

byte hexToByte(String hex){
  return ((hexCharToNum(hex.charAt(0))<<4) + (hexCharToNum(hex.charAt(1)))); //bitshift <<4 doesnt work
}
byte hexCharToNum(char letter){
  switch(letter){ //could do better with an ascii representation
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a': case 'A': return 10;
    case 'b': case 'B': return 11;
    case 'c': case 'C': return 12;
    case 'd': case 'D': return 13;
    case 'e': case 'E': return 14;
    case 'f': case 'F': return 15;
    default:
      Serial.println("Unknown value: " + String(letter)); 
      return 0;
  }
}
