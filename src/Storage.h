#ifndef STORAGE_H
#define STORAGE_H

#include <ArduinoJson.h>
#include <LittleFS.h>

class Storage
{
public:
    void init();
    void writeJSON(const String& filename, const String& jsonContent);
    String readJSON(const String& filename);
    void updateJSON(const String &filename, const String &key, const String &newValue);
private:
};


#endif

extern Storage storage;