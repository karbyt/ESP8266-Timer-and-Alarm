#include <Arduino.h>
#include "WifiSettings.h"
#include "Buzzer.h"
#include "Audio.h"
#include "Storage.h"
#include "WebServer.h"
#include "TimeManager.h"
#include "TimerManager.h"
#include "AlarmManager.h"
#include "NTPClient.h"
#include "Relay.h"

extern AsyncWebServer server;

void setup()
{
    Serial.begin(115200);
    audio.init();
    buzzer.init();
    storage.init();
    wifiSettings.init();
    timerManager.init();
    relay.init();
    alarmManager.init(&timeManager);
    webserver.init();
}

void loop()
{
    timeManager.update();
    alarmManager.checkAlarms();
    timerManager.update();
    relay.update();
    // wifiSettings.handleClient(); // Add this line to handle WiFi manager clients
}