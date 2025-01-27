// TimerManager.h
#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "Storage.h"

class TimerManager
{
private:
    struct RelayConfig
    {
        int id;
        unsigned long on_time;
        unsigned long off_time;
        int cycle;
        bool reversed;
        int initial_cycle;
        unsigned long startTime;  // Waktu mulai untuk timer ini
    };

    RelayConfig relayConfigs[4];
    int relayPins[4];
    bool isRunning;
    int currentCycle[4];
    bool relayState[4];
    unsigned long baseTime;  // Waktu referensi untuk semua timer

    void saveRunningState();
    bool loadRunningState();
    void saveConfig();
    void loadConfig();
    void initializeRelayStates();
    void resetConfig();
    void resetCycles();
    void updateRelay(int index);

public:
    TimerManager();
    void init();
    void update();
    void start();
    void stop();
    bool addTimer(int id, unsigned long onTime, unsigned long offTime, int cycleCount, bool reversed);
    bool updateTimer(int id, unsigned long onTime, unsigned long offTime, int cycleCount, bool reversed);
    bool deleteTimer(int id);
    bool getIsRunning() { return isRunning; }
};

extern TimerManager timerManager;

#endif