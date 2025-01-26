#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <Arduino.h>
#include <NTPClient.h>

class TimeManager
{
public:
    void init();
    void update();
    String getTimeJson();
    int getHours();
    int getMinutes();
    int getSeconds();
    int getDay();
    bool isTimeSet();
};
#endif

extern TimeManager timeManager;
extern NTPClient timeClient;