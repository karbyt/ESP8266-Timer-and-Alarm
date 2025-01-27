#ifndef WEBSERVER
#define WEBSERVER

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>


#include <LittleFS.h>
#include "Storage.h"

class Logger
{
public:
    void init(AsyncWebServer &server);
    void addLog(const String &log);

private:
    String logBuffer;
    const size_t maxLogSize = 1024;
};

class Webserver
{
public:
    void init();

private:
    void setupServerRoutes();
};

#endif

extern Webserver webserver;
extern Logger logger;