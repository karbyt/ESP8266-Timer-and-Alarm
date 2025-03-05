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
    void handleChunkedJsonRequest(AsyncWebServerRequest *request, const String &filename);
};

#endif

extern Webserver webserver;
