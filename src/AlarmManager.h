// AlarmManager.h
#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <ArduinoJson.h>
#include <vector>
#include "TimeManager.h"
#include "Audio.h"
#include "Buzzer.h"
#include "Relay.h"

class AlarmManager
{
public:
    void init(TimeManager *timeManager);
    void checkAlarms();
    void addAlarm(const String &label, const String &time, String days, int ringtone, const String &relay);
    bool updateAlarm(int id, String label, String time, String days, int ringtone, String relay);
    void removeAlarm(int id);
    String getAlarmsJson();
    void processRelayString(const String &relayStr);

private:
    struct RelayTimer
    {
        byte relayNumber;
        unsigned long startTime;
        unsigned long duration;
    };

    TimeManager *_timeManager;
    std::vector<RelayTimer> activeRelays;
    bool alarmTriggeredThisMinute = false;

    void updateRelayTimers();
};

extern AlarmManager alarmManager;

#endif