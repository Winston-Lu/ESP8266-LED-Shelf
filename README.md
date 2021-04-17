# LED-Clock

An improved version of [this LED shelf by DIY Machines on Youtube](https://www.youtube.com/watch?v=8E0SeycTzHw) to include more features and settings.

## My Improvements over the original:
* Used an ESP8266 instead of an Arduino Nano
* Auto-update time from the internet (No need for a real-time clock or manual configuration aside from UTC offsets)
* Created a webserver to control different settings:
  * Toggle On/Off
  * Control Brightness (Segments, background, or spotlight dimming)
  * Modify Time (for daylight savings)
  * Color Control
  * Pattern Control
  * 24hr clock support (Untested)
* It also includes different patterns/lighting effects
  * Static/Solid
  * Rainbow
  * Gradient  
  * Sparkle
  * Rain (WiP)
* Every segment on the shelf has LEDs
  * Requires exactly 300 WS2812B LEDs (compared to the original 219)
    * You can buy a 5m 60 LED's/m WS2812B LEDS strip for about $20 CAD on Aliexpress. Just pray that none of the LED's come dead on arrival
  * 32 Segments (compared to the original 23)
  * 12 Spotlights
* Customizable to 24-hour format
  * (Untested) Requires a few configuration modifications


## Configuration
### In Config.h
Variable | Description
---------|---------
LEDS_PER_LINE        | Should be 9 unless you want to use 30 LEDs/m or 144 LEDs/m
MILLI_AMPS           | Amperage rating of the power supply. I used a 12v 3A supply and used 2 buck converters to step down. Wired half of the LEDs to the first buck converter, and half to the 2nd. I do not really recommend doing it like this since it does heat up quite a bit when it runs at 100% brightness on white (~90c) which could deform PLA. I have a heatsink on it to reduce heat, but the lack of airflow is concerning
FRAMES_PER_SECOND    | Shouldn't be above 30 or else some timing stuff may lag behind. If you don't mind, you can set to 60
\_12_HR_CLOCK         | 12-hour clock layout
\_24_HR_CLOCK         | 24-hour clock layout. Requires additional segments and LEDs
LIGHT_SENSOR         | Should be connected to A0, since thats the only analog pin on the board
DATAPIN              | Connected to D8 on my NodeMCU v0.9 (pin 15)
LED_TYPE             | Should be WS2812B's in most cases
COLOR_ORDER          | WS2812B's are GRB. If colors act weird or you are using other LED_TYPE's, you may need to switch to RGB
NAME                 | Name of the ESP8266 device (used to connect or as an identifier so you know what device is which in the routers "device" page
EEPROM_UPDATE_DELAY  | How many seconds to wait after a change before saving it to EEPROM. This is to reduce writes to EEPROM so we don't wear it out
segmentWiringOrder[] | Compensates for the difference between how the code references segments and how its actually wired. The wiring order goes from 1-2-3-4-5-6-7... in the order it is wired. If the direction of the wiring points towards the bottom right, the number is positive. If the direciton of the wiring points towards the upper left, the direction is negative. This is to both make effects easier without needing to hard-code any values. The numbers in the array represent the position of the segment it covers. The way I wired it was starting at segment 7 moving upwards, then moving right across segment 1, then down to segment 8, right to segment 14, and so on.
spotlightWiringOrder[] | Same as above, but for spotlights. Starts at 0 this time since we don't need +- signs.
m_one[] | segment indicies for the ones digit for the minutes
m_ten[] | segment indicies for the tens digit for the minutes
h_one[] | segment indicies for the ones digit for the hours
h_ten[] | segment indicies for the tens digit for the hours

## In Secrets.h
Variable | Description
---------|---------
ssid | Name of the WiFi point to connect to. Otherwise the clock will not sync and you can't connect to the webserver
password | password to the WiFi point

## In WebServer.cpp
Variable | Description
---------|---------
ip | Static IP Configuration so you can connect using a 192.168.#.# address
gateway | IP address of your router. You can type "ipconfig" on Windows or "ifconfig" on Mac/Linux to find this IP. Usually it is 192.168.0.1 or 192.168.1.1 for home networks

## Webserver configuration
Command | Description | Example
---------------|----------|----------
IP/cmd?c=utcoffset&v=### | Replace ### with your UTC offset. This only needs to be done once, and is saved on reset | http://192.168.1.51/cmd?c=utcoffset&v=-8 
IP/cmd?c=rainbowrate&v=### | Replace ### with the rate. By default, this value is 5 | http://192.168.1.51/cmd?c=rainbowrate&v=3
IP/cmd?c=reset | Factory Reset settings, including UTC offset | http://192.168.1.51/cmd?c=reset
IP/cmd?c=resetProfile | Factory Reset lighting settings, but doesnt reset UTC offset | http://192.168.1.51/cmd?c=resetProfile


## Creating
Add pictures later
Heres a total Bill of Materials and tools I used for my setup
* A 3d printer. Any cheap printer would work, but it would need a bed size of at least 150mm x 150mm to print the wall segments
* Screwdrivers and/or a drill helps
* Tweezers to run wires through the bracket holes
* 1kg of filament for the 31 + 1 brackets, as well as the diffusers. I recommend White filament (I used PLA, but PETG or ABS is fine too)
  * You can skimp out on infill (~10-15%) for the brackets, but the load bearing ones should be higher
  * I would print them lying on the side, as we dont really care about overhangs and layer lines would be perpendicular to the load
    * DIY Machines printed them vertically, but with weaker filaments like PolyLite, printing horizontally would be much better
    * That way the load will be going perpendicular to the layer lines (The point that usually fails first), while the almost non-existant transverse strain will go against the layer lines
    * Overhangs won't look too great, but we won't be looking at them
* 3kg of wood or brown filament (PETG or PLA works). This is for the shelf covers. You will use about 2.2kg, not including any failed prints
* A Wood panel. I used a 3/4in x 2ft x 4ft piece of Plywood from Home Depot. I wouldn't recommend anything smaller for a 12hr shelf
* >64 #8 Wood screws. 64 would be the minimum (2 per segment)
* Cabinet/Painting mounting bracket to mount to the wall
* A handful of wire, preferably of red, black, +1 other color. Buying a set from Amazon will be more than enough
* A soldering iron and some solder
* A LED power supply (Recommended >40w power supply). The LED's are 5v, so I would go with a 5v power supply. If you go with a 12v, you must use 12v LEDs or you must step down to 5v using a step-down buck converter
* An ESP8266. You can get these on Amazon or Aliexpress.
* 5m 60 LEDs/m Addressable RGB LED strip. You can go with a 30 LED/m strip, but you will have some dark spots, or you can go with 144 LED/m with a 2.4x increase in power usage for the tradeoff of having really bright lights such that it will probably shine through the shelf walls. 
* A computer to upload code with a micro USB cable

Optional Materials
* Terminal blocks if you don't want to solder as much
* Photoresistor + 10k ohm resistor if you want autobrightness
* Buck converters if you want to use an existing laptop power supply to step down to 5v (I would recommend at least 2 or 3. 2 buck converters start to overheat at 100% load after 45 minutes or so)

### Printing
Each wall segment takes about 3 hours to print each. I would print them horizontally and not vertically as shown in the original video to get stronger prints
Each wall takes about 8 hours to print each. The first 3 layers are printed in white PLA, while the rest is with wooden PLA (or whatever plastic you use)
You only need 1 of the layout template. Remember to measure twice and drill once
The STL files for these parts are in the original video description, so give him some support by going to the video.

### The ESP8266 Part
Run this code in some folder you want to save this code if you have git installed. Otherwise just download zip in the top left and extract:
```cmd
git clone https://github.com/Winston-Lu/LED-Clock
```
1. Download and open the [Arduino IDE](https://www.arduino.cc/en/software) 
2. Go to "File" > "Preferences"
3. Under "Additional Boards Manager URLs:", add `http://arduino.esp8266.com/stable/package_esp8266com_index.json`. If you have entries there already, add a comma to seperate them
4. Click OK and go to "Tools" > "Board" > "Board Manager"
5. Look up "esp8266", then install the board
6. Once it's installed, go to "Tools" > "Board" > "ESP8266 Boards" > "Generic ESP8266 Module". This will work for the NodeMCU v0.9 and NodeMCU v1.0.
7. Change flash size to "4MB (FS: 1MB OTA ~1019MB)"
8. Once you plugged in your ESP8266, select the COM port it is connected to
9. Leave every other setting default
10.  Download the [LittleFS tool from this repository here](https://github.com/earlephilhower/arduino-esp8266littlefs-plugin/releases)
11.  Follow their installation instructions and under "Tools", you should see "ESP8266 LittleFS Data Upload"
12.  Click on it and let it upload the webserver code
13.  After that, you should be done.

### Wiring
//Add picture of my wiring direction
On the ESP8266, I soldered one of the LED strip 3-pin male header onto it so I can hot-swap the module with only 1 connection needed. The red wire goes into Vin, white wire goes into G (ground), and the green wire into D8. I also have a photoresistor connected to A0 and 3v (actually 3.3v), as well as a 10k ohm resistor from A0 to G as a pull-down resistor. if you do not have this resistor, the analog readings will not be accurate.

//Add picture of ESP8266 wiring

Connect 2 red wires to the 5V line on the LED strip, one connected to the 5v power supply and 1 to the LED strip female header red wire. Do the same for the black wires: 2 coming from the GND pad on the LED strip, one to GND on the power supply and 1 to the GND pin on the female LED strip header. For data, we only need 1 wire coming from the LED strip data line into the female header (usually in the middle). Once you get that setup, you should be able to hotswap the module in case you want to do any testing and don't want to setup Arduino OTA uploads. 



## Adding Effects
Effects are rather hard to create since we're working with segment indicies going in different directions.
Since this doesnt translate nicely to a 2D array, most effects I've made relies a lot on regression, so it makes
the code very difficult to read and initially understand.

I have a few functions that make creating effects easier
Function | Description
---------|---------
strip segmentToLedIndex() | Returns a `strip` struct, which contains the first LED index (lowest index) and if the effect should be reversed (pointing to the top left rather than bottom right). If the first LED index is 27, strip is in reverse, and LEDS_PER_LINE is 9, then the LEDs are \[27,28,29,30,31,32,33,34,35] moving up or to the right
int spotlightToLedIndex() | Returns the LED index in the `leds[]` array of a spotlight

