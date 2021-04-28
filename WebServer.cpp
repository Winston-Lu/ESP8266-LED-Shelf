#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#include <FastLED.h>
#include <LittleFS.h> //for file system
#include "Config.h"
#include "WebServer.h"
#include "Secrets.h"
#include "Lighting.h"
#include "NTPTime.h"

//IP config
IPAddress ip(192,168,1,51);     //Device IP
IPAddress gateway(192,168,1,1); //IP of router
IPAddress subnet(255,255,255,0);
IPAddress primaryDNS(8,8,8,8);
IPAddress secondaryDNS(8,8,4,4);

ESP8266WebServer webServer(80);
ESP8266HTTPUpdateServer httpUpdateServer;

void setupWiFi(){
  if (!WiFi.config(ip, gateway, subnet, primaryDNS, secondaryDNS)) {Serial.println("STA Failed to configure");}  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status()!=WL_CONNECTED){delay(500);Serial.print(".");}
  Serial.println();
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
}

void updateServer(){
  webServer.handleClient();
  MDNS.update();
  
  static bool hasConnected = false;
  EVERY_N_SECONDS(1) {
    if (WiFi.status() != WL_CONNECTED) {hasConnected = false;}
    else if (!hasConnected) {
      hasConnected = true;
      MDNS.begin(NAME);
      MDNS.setHostname(NAME);
      webServer.begin();
      Serial.println("HTTP web server started");
      Serial.print("Connected! Open http://");
      Serial.print(WiFi.localIP());
      Serial.print(" or http://");
      Serial.print(NAME);
      Serial.println(".local in your browser");
    }
  }
}

void setupServer(){
  httpUpdateServer.setup(&webServer);

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
    setForegroundTransparency((byte)webServer.arg("value").toInt());
    lightingChanges.foregroundTransparency = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/autobrightness", HTTP_GET, []() {
    autobrightness = (webServer.arg("value")=="true");
    lightingChanges.autobrightness = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/brightness", HTTP_GET, []() {
    setSegmentBrightness((byte)webServer.arg("value").toInt());
    lightingChanges.segmentBrightness = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/spotlightbrightness", HTTP_GET, []() {
    setSpotlightBrightness((byte)webServer.arg("value").toInt());
    lightingChanges.spotlightBrightness = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/backgroundbrightness", HTTP_GET, []() {
    setBackgroundBrightness((byte)webServer.arg("value").toInt());
    lightingChanges.backgroundBrightness = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });
  
  webServer.on("/foregroundTransparency", HTTP_GET, []() {
    setForegroundTransparency((byte)webServer.arg("value").toInt());
    lightingChanges.foregroundTransparency = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/bgcolor", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    color.red = webServer.arg("red").toInt();
    color.green = webServer.arg("green").toInt();
    color.blue = webServer.arg("blue").toInt();
    bg = color;
    lightingChanges.bg = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/bgcolor2", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    color.red = webServer.arg("red").toInt();
    color.green = webServer.arg("green").toInt();
    color.blue = webServer.arg("blue").toInt();
    bg2 = color;
    lightingChanges.bg2 = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/h1color", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    color.red = webServer.arg("red").toInt();
    color.green = webServer.arg("green").toInt();
    color.blue = webServer.arg("blue").toInt();
    h_ten_color = color;
    lightingChanges.h_ten_color = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/h2color", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    color.red = webServer.arg("red").toInt();
    color.green = webServer.arg("green").toInt();
    color.blue = webServer.arg("blue").toInt();
    h_one_color = color;
    lightingChanges.h_one_color = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  
  webServer.on("/m1color", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    color.red = webServer.arg("red").toInt();
    color.green = webServer.arg("green").toInt();
    color.blue = webServer.arg("blue").toInt();
    m_ten_color = color;
    lightingChanges.m_ten_color = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/m2color", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    color.red = webServer.arg("red").toInt();
    color.green = webServer.arg("green").toInt();
    color.blue = webServer.arg("blue").toInt();
    m_one_color = color;
    lightingChanges.m_one_color = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });
  
  webServer.on("/spotcolor", HTTP_GET, []() {
    CRGB color = CRGB::Black;
    int index = webServer.arg("value").toInt();
    color.red = webServer.arg("red").toInt();
    color.green = webServer.arg("green").toInt();
    color.blue = webServer.arg("blue").toInt();
    setSpotlightColor(index-1,color);
    if(index==1) setSpotlight1(color);
    else if(index==2) setSpotlight2(color);
    lightingChanges.spotlights[index-1] = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/effectfg", HTTP_GET, []() {
    String pattern = webServer.arg("value");
    if(pattern == "off") foregroundPattern = 0;
    else if(pattern == "solid") foregroundPattern = 1;
    else if(pattern == "rainbow") foregroundPattern = 2;
    else if(pattern == "gradient") foregroundPattern = 3;
    lightingChanges.foregroundPattern = true;
    lastUpdate = 0;
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
    lightingChanges.backgroundPattern = true;
    lastUpdate = 0;
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
    lightingChanges.spotlightPattern = true;
    lastUpdate = 0;
    updateSettings = true;
    webServer.send(200, "text/plain", "1");
  });

  webServer.on("/utcoffset", HTTP_GET, []() {
    storeUtcOffset(webServer.arg("value").toDouble());
    setNewOffset();
    webServer.send(200, "text/plain", "1");
  });
  
  webServer.on("/cmd", HTTP_GET, []() {
    String command = webServer.arg("c");
    String val = webServer.arg("v");
    if(command=="help" || command=="?"){
      webServer.send(200, "text/plain", String("Available commands:<br>" 
                                        "utcoffset ## - Set UTF offset in hours for clock [current offset: " + String(getOffset()) + "]<br>" +
                                        "rainbowrate ## - Set how rainbowy the rainbow is [current rate: " + String(rainbowRate) + "]<br>" +
                                        "reset - Reset all settings<br>" +
                                        "resetprofile - Reset lighting settings"
                                        ));
    }else if(command=="utcoffset"){
      utcOffset = val.toDouble();
      setNewOffset();
      storeUtcOffset(utcOffset);
      webServer.send(200, "text/plain", "Set UTC offset to " + String(getUtcOffset()));
    }else if(command=="rainbowrate"){
      rainbowRate = (double)val.toInt();
      lightingChanges.rainbowRate = true;
      lastUpdate = EEPROM_UPDATE_DELAY*FRAMES_PER_SECOND;
      updateSettings = true;
      webServer.send(200, "text/plain", "Set rainbow rate to " + String(rainbowRate));
    }else if(command=="reset"){
      defaultSettings();
      saveAllSettings();
      webServer.send(200, "text/plain", "Reset Settings");
    }else if(command=="resetprofile"){
      deleteSettings();
      webServer.send(200, "text/plain", "Profile will reset to default on reboot");
    }else{
      webServer.send(200, "text/plain", "Command not found");
    }
  });


  webServer.serveStatic("/", LittleFS, "/", "max-age=86400");

  MDNS.begin(NAME);
  MDNS.setHostname(NAME);

  webServer.begin();
  Serial.println("HTTP web server started");
}

void sendInt(uint8_t value){sendString(String(value));}
void sendString(String value){webServer.send(200, "text/plain", value);}
void broadcastInt(String name, uint8_t value){String json = "{\"name\":\"" + name + "\",\"value\":" + String(value) + "}";}
void broadcastString(String name, String value){String json = "{\"name\":\"" + name + "\",\"value\":\"" + String(value) + "\"}";}
