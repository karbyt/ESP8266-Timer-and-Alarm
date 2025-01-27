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

const int led = 2;
const int hour = 18;
const int minute = 5;

void setup()
{
    Serial.begin(115200);
    audio.init();
    buzzer.init();
    storage.init();
    wifiManager.init();
    timeManager.init();
    alarmManager.init(&timeManager);
    webserver.init();
    relay.init();
    timerManager.init();
}

void loop()
{
    timeManager.update();
    alarmManager.checkAlarms();
    timerManager.update();
}
