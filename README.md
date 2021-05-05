# ESP8266 LED Shelf

An improved version of [this LED shelf by DIY Machines on Youtube](https://www.youtube.com/watch?v=8E0SeycTzHw) to include more features and settings.

### Work in progress, images are not the final product

<img src="https://user-images.githubusercontent.com/33874247/117093146-21acb680-ad2e-11eb-9009-c75ef6d17bf0.png" width="350" height="420" />
<img src="https://user-images.githubusercontent.com/33874247/117094072-c3350780-ad30-11eb-89fc-b0ca06d4eca0.jpg" width="600" height="250" />
<img src="https://user-images.githubusercontent.com/33874247/117094138-fd060e00-ad30-11eb-81cd-c64c8d04d459.jpg" width="600" height="250" />


## My Improvements over the original:
* Used an ESP8266 instead of an Arduino Nano
* Auto-update time from the internet (No need for a real-time clock or manual configuration aside from UTC offsets)
* Created a webserver to control different settings:
  * Toggle On/Off
  * Control Brightness (Segments, background, or spotlight dimming)
  * Modify Time (for daylight savings)
  * Color Control
  * Pattern Control
* It also includes different patterns/lighting effects
  * Static/Solid
  * Rainbow
  * Gradient  
  * Sparkle
  * Rain/Snow
  * More to be implemented
* Every segment on the shelf has LEDs
  * Requires exactly 300 WS2812B LEDs (compared to the original 219) for the 12hr version. Would need 347 total for a 24hr version (including spotlights)
    * You can buy a 5m 60 LED's/m WS2812B LEDS strip for about $20 CAD on Aliexpress. Just pray that none of the LED's come dead on arrival
  * 32 Segments for the 12hr version (compared to the original 23). 37 Segments for the 24hr version
  * 12 Spotlights for the 12hr version. 14 Spotlights for the 24hr version
* Support for 24-hour format
  * (Mostly untested) Requires some configuration modifications (See **Setting to 24hr layout** below). Should work fine since the code would break for the 12hr version if the 24hr version wouldn't also work due to the less readable but modular code
* Expandable to larger shelf sizes
  * (Untested) The Arduino code is much more modular than the web page. Setting sizes in `Config.h` should work, but the webpage support for solid spotlights is not. The Arduino code should still work fine, but I don't have the resources to test this.

This should work if you decide to not add spotlight LED's. I kept the more common configuration changes such as lighting effects easily accessable from the web-server, while less common configurations such as changing UTF offset for daylight savings for timezones or setting FPS as a webserver command. Other typically non-changing variables such as the # of LEDS, display width/height, and others are coded in `Config.h`. The configurations are persistent on restart, so all effects will be saved on a power loss or restart.


## Configuration

## Setting to 24hr layout (Optional)
To switch to 24hr mode, you need to change a line in `./data/script.js` website file and on the first line, change it to `const enable24HR = true;`. This will allow for an extra column of spotlights if you added them. If you have a setup with a setup larger than >2x7, then you will need to code in the spotlight modification yourself. I didn't make this as modular as I would have liked as I wanted to avoid using frameworks like AngularJS ng-repeat since I'm not the best front-end designer.

In the C++ code, comment out \_12_HR_CLOCK and uncomment \_24_HR_CLOCK in `Config.h`. Width and Height should automatically reconfigure to support this. If you wired everything exactly as I did (see image below), you don't need to make any more configuration changes in the C++ code. Otherwise, follow the steps below.

Last thing you may need do is to go into `Config.h` and modify the segmentWiringOrder, spotlightWiringOrder, h_ten[], h_one[], m_ten[], and m_one[]. The first array tells the program how you wired the clock, so each number in the array is the lighting index that segment covers. The numbers in those last 4 arrays should be the lighting (not wiring) index. The image below is the wiring order that is coded into the program right now. However, if your wiring order is not the same, you have to reference in what order your segments are wired. See the code in `Config.h` to learn in what order to put it in.

![Wiring](https://user-images.githubusercontent.com/33874247/117093218-4ef96480-ad2e-11eb-81ca-bb69942681c6.png)

## Setting up Secrets.h (Required)
Create a file called "Secrets.h" with the following code. Replace the placeholder text with the relevant information:
```c++
const char PROGMEM *ssid     = "Your WiFi Name Here";
const char PROGMEM *password = "Your WiFi Password Here";
```
Variable | Description
---------|---------
ssid | Name of the WiFi point to connect to. Otherwise the clock will not sync and you can't connect to the webserver
password | Password to the WiFi point

## Webserver configuration (Recommended to set UTC Offset)
At the bottom of the webpage once you have everything setup, there is a spot for commands at the bottom of the page:

Command | Description | Usage
---------------|----------|----------
utcoffset | Sets your UTC offset in hours. This only needs to be done once, and is saved on reset in EEPROM | `utcoffset -8 `
rainbowrate | This changes how rainbow-y the rainbow is. By default, this value is 5. This value will be saved when the device restarts.  | `rainbowrate 3`
fps | Set the frames per second of the display. Default is 30, I don't think its fast enough to do 60. All effect speeds are fps-dependent. This is not saved on restart, as this is more of a debug feature currently | `fps 24`
reset | Factory Reset settings, including UTC offset | `reset`
resetprofile | Factory Reset lighting settings, but doesnt reset UTC offset | `resetProfile`
loading | Plays the loading effect. This plays when the shelf is connecting to WiFi, but if you like it, you can set it here | `loading`


### Config.h (Mostly required)
This is to modify some settings in case your setup is not wired exactly like mine. I've bolded any settings that you would most likely need to modify

Variable | Description
---------|---------
LEDS_PER_LINE        | Should be 9 unless you want to use 30 LEDs/m or 144 LEDs/m
**MILLI_AMPS**           | Amperage rating of the power supply. I used a 12v 3A supply and used 2 buck converters to step down. Wired half of the LEDs to the first buck converter, and half to the 2nd. I do not really recommend doing it like this since it does heat up quite a bit when it runs at 100% brightness on white (~90c) which could deform PLA. I have a heatsink on it to reduce heat, but the lack of airflow is concerning
FRAMES_PER_SECOND    | Shouldn't be above 30 or else some timing stuff may lag behind. If you don't mind, you can set to 60
\_12_HR_CLOCK         | 12-hour clock layout. This is selected by default
\_24_HR_CLOCK         | 24-hour clock layout. Requires additional segments and LEDs. See **Setting to 24hr layout** above to change the layout to 24hr
LIGHT_SENSOR         | Should be connected to A0, since thats the only analog pin on the board
**DATAPIN**              | Connected to D8 on my NodeMCU v0.9 (pin 15)
LED_TYPE             | Should be WS2812B's in most cases
COLOR_ORDER          | WS2812B's are GRB. If colors act weird or you are using other LED_TYPE's, you may need to switch to RGB
NAME                 | Name of the ESP8266 device (used to connect or as an identifier so you know what device is which in the routers "device" page
EEPROM_UPDATE_DELAY  | How many seconds to wait after a change before saving it to EEPROM. This is to reduce writes to EEPROM so we don't wear it out
**segmentWiringOrder[]** | Compensates for the difference between how the code references segments and how its actually wired. The wiring order goes from 1-2-3-4-5-6-7... in the order it is wired. If the direction of the wiring points towards the bottom right, the number is positive. If the direciton of the wiring points towards the upper left, the direction is negative. This is to both make effects easier without needing to hard-code any values. The numbers in the array represent the position of the segment it covers. The way I wired it was starting at segment 7 moving upwards, then moving right across segment 1, then down to segment 8, right to segment 14, and so on.
**spotlightWiringOrder[]** | Same as above, but for spotlights. Starts at 0 this time since we don't need +- signs. If you don't have any spotlights, just leave this alone
**m_one[]** | segment indicies for the ones digit for the minutes
**m_ten[]** | segment indicies for the tens digit for the minutes
**h_one[]** | segment indicies for the ones digit for the hours
**h_ten[]** | segment indicies for the tens digit for the hours

## In WebServer.cpp (Mostly Required)
You may need to change the settings if you use a 192.168.0.# local IP address or want to change to a different static IP address. I would not recommend using DHCP since I usually find more success connecting via IP address instead of the hostname (you would need to modify more code if you wanted to use DHCP anyways). To find your IP, you can type "ipconfig" on Windows in cmd or "ifconfig" on Mac/Linux in terminal to find this IP. Usually it is 192.168.0.1 or 192.168.1.1 for home networks.
Variable | Description
---------|---------
ip | Static IP Configuration so you can connect using a 192.168.#.# address
gateway | IP address of your router
subnet | Subnet address of your home address. If you don't know what this is, you probably wont need to touch it



## Creating
Add pictures later
Heres a total Bill of Materials and tools I used for my setup
* A 3D printer. Any cheap printer would work, but it would need a bed size of at least 150mm x 150mm to print the wall segments. The wall piece is about 180mm long
* Screwdrivers and/or a drill helps
* Tweezers to run wires through the bracket holes
* 1kg of filament for the 31 + 1 brackets, as well as the diffusers. I recommend White filament (I used PLA, but PETG or ABS is fine too)
  * You can skimp out on infill (~10-15%) for the brackets, but the load bearing ones should be higher
  * I would print them lying on the side, as we dont really care about overhangs and layer lines would be perpendicular to the load
    * DIY Machines printed them vertically, but with weaker filaments like PolyLite, printing horizontally would be much better
    * That way the load will be going perpendicular to the layer lines (The point that usually fails first), while the almost non-existant transverse strain will go against the layer lines
    * Overhangs won't look too great, but we won't be looking at them
* 3kg of wood or brown filament (PETG or PLA works). This is for the shelf covers. You will use about 2.6kg, not including any failed prints/filament change purges
* A Wood panel. I used a 3/4in x 2ft x 4ft piece of Plywood from Home Depot. I wouldn't recommend anything smaller for a 12hr shelf
* \>64 #8 Wood screws. 64 would be the minimum (2 per segment)
* Cabinet/Painting mounting bracket to mount to the wall
* A handful of wire, preferably of red, black, +1 other color. Buying a set from Amazon will be more than enough
* A soldering iron and some solder
* A LED power supply (Recommended >40w power supply). The LED's are 5v, so I would go with a 5v power supply. If you go with a 12v, you must use 12v LEDs or you must step down to 5v using a step-down buck converter
* An ESP8266. You can get these on Amazon or Aliexpress.
* 5m 60 LEDs/m Addressable RGB LED strip. You can go with a 30 LED/m strip, but you will have some dark spots, or you can go with 144 LED/m with a 2.4x increase in power usage for the tradeoff of having really bright lights such that it will probably shine through the shelf walls. 
* A computer to upload code with a micro USB cable
* Hot glue gun and/or adhesives 

Optional Materials
* Terminal blocks if you don't want to solder as much
* Photoresistor + 10k ohm resistor if you want autobrightness
* Buck converters if you want to use an existing laptop power supply to step down to 5v (I would recommend at least 2 or 3. 2 buck converters start to overheat at 100% load after 45 minutes or so)

### Printing
* Each wall segment takes about 3 hours to print each. I would print them horizontally and not vertically as shown in the original video to get stronger prints
* Each wall takes about 8 hours to print each. The first 3 layers are printed in white PLA, while the rest is with wooden PLA (or whatever plastic you use)
* You only need 1 of the layout template. Remember to measure twice and drill once
* The STL files for these parts are in the original video description, so give him some support by going to the video.

### The ESP8266 Part
Run this code in some folder you want to save this code if you have git installed. Otherwise just download the repo zip in the top right and extract:
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
8. Once you plugged in your ESP8266, select the COM port it is connected to. You can leave every other setting default
9.  Download the [LittleFS tool from this repository here](https://github.com/earlephilhower/arduino-esp8266littlefs-plugin/releases)
10.  Follow their installation instructions and under "Tools", you should see "ESP8266 LittleFS Data Upload"
11.  Click on it and let it upload the webserver code
12.  Create a `Secrets.h` file and modify the settings accordingly. Read above for how to configure your Wi-Fi settings and other important settings.
13.  Upload the Arduino code by clicking the right-facing arrow near the top left. 
14.  After that, you should be done. Plug the ESP8266 in and start configuring some web settings such as the UTC offset by going to the webserver (Default http://LEDShelf.local or 192.168.1.51), scrolling to the bottom, and typing in the `utcoffset` command. This is meant to be configured online since daylight savings would make updating this a hassle

### Wiring
<img src="https://user-images.githubusercontent.com/33874247/117094177-10b17480-ad31-11eb-9f2e-6b3de03d06f2.jpg" width="600" height="350" />
On the ESP8266, I soldered one of the LED strip 3-pin male header onto it so I can hot-swap the module with only 1 connection needed. The red wire goes into Vin, white wire goes into G (ground), and the green wire into D8. I also have a photoresistor connected to A0 and 3v (actually 3.3v), as well as a 10k ohm resistor from A0 to G as a pull-down resistor. if you do not have this resistor, the analog readings will not be accurate.

Connect 2 red wires to the 5V line on the LED strip, one connected to the 5v power supply and 1 to the LED strip female header red wire. Do the same for the black wires: 2 coming from the GND pad on the LED strip, one to GND on the power supply and 1 to the GND pin on the female LED strip connector. For data, we only need 1 wire coming from the LED strip data line into the female connector (usually in the middle). Once you get that setup, you should be able to hotswap the module in case you want to do any testing and don't want to setup Arduino OTA uploads. If you don't want to hotswap, you can just solder directly onto the board's VIN, GND, and D8



## Adding Effects
Effects are rather hard to create since we're working with segment indicies going in different directions.
Since this doesnt translate nicely to a 2D array, most effects I've made relies a lot on regression, so it makes
the code very difficult to read and initially understand.

All effect rendering is done in the `Lighting.cpp` file, usually in the `showLightingEffects()` function near the top.

I have a few functions that make creating effects easier
Function | Description
---------|---------
strip segmentToLedIndex() | Returns a `strip` struct, which contains the first LED index (top-left most LED index) and if the effect should be reversed (pointing to the top left rather than bottom right, so count down rather than up). If the first LED index is 27, strip is in reverse, and LEDS_PER_LINE is 9, then the LEDs are \[27,26,25,24,23,22,21,20,19] moving up or to the right
int spotlightToLedIndex() | Returns the LED index in the `leds[]` array of a spotlight
struct grid2d | Meant to represent the lights in a more 2d way split into a vertical and horizontal segment portion. Used in the rain effect



## To Do

### Lighting
1. Create a fire effect

### Functional (Based on threads in the original repo)
1. Optional hyphen seperator for hours and minutes
2. DHT22 temperature & humidity sensor support
3. MQTT support
