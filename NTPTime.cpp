#include "Config.h"
#include "NTPTime.h"
#include "Lighting.h"

#include <WiFiUdp.h>
#include <NTPClient.h>


double utcOffset;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void initClock(){
  timeClient.begin();
  utcOffset = getUtcOffset();
  setNewOffset();
  updateTime();
  Serial.println("Time updated");
}
void updateTime(){timeClient.update();}
void setNewOffset(){timeClient.setTimeOffset((int)(utcOffset*3600));}
double getOffset(){return utcOffset;}
int getHour12(){return ((timeClient.getHours()%12 + 12*(timeClient.getHours()%12==0)));} //branchess formula to get 12 to display, but cycle back to 1 in a 12hr format instead of displaying 0 when it is 12am/pm
int getHour24(){return (timeClient.getHours());}
int getMinute(){return (timeClient.getMinutes());}
int getSecond(){return (timeClient.getSeconds());}
