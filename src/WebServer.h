#ifndef WEBSERVER
#define WEBSERVER

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>


#include <LittleFS.h>
#include "Storage.h"


class Webserver
{
public:
    void init();

private:
    void setupServerRoutes();
};

#endif

extern Webserver webserver;
