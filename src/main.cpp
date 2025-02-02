#include "Buzzer.h"
#include "WifiManager.h"
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
    wifiManager.init();
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
}
