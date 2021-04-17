#ifndef NPTTIME_H
#define NPTTIME_H
extern double utcOffset; 
void initClock();
void updateTime();
void setNewOffset();
int getHour12();
int getHour24();
int getMinute();
int getSecond();
#endif
