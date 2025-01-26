// AlarmManager.h
#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <ArduinoJson.h>

#include "TimeManager.h"
#include "Audio.h"
#include "Buzzer.h"

class AlarmManager {
public:
    void init(TimeManager* timeManager);
    void checkAlarms();
    void addAlarm(const String &label, const String &time, String days, int ringtone, const String &relay, int relayDuration);
    bool updateAlarm(int id, String label, String time, String days, int ringtone, String relay, int relayDuration);
    void removeAlarm(int id);
    String getAlarmsJson();

private:
    TimeManager* _timeManager;
    bool _alarmTriggered = false;
    unsigned long _lastTriggerTime = 0;
};

#endif

extern AlarmManager alarmManager;