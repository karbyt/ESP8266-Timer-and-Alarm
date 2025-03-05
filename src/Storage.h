#ifndef STORAGE_H
#define STORAGE_H

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>

class Storage
{
public:
    void init();
    void writeJSON(const String& filename, const String& jsonContent);
    String readJSON(const String& filename);
    void updateJSON(const String &filename, const String &key, const String &newValue);
    bool serveFile(const String &filename, AsyncWebServerRequest *request, const String &contentType, const String &contentEncoding = "");
private:
};


#endif

extern Storage storage;