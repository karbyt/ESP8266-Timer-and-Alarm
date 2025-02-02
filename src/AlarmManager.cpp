// AlarmManager.cpp
#include "AlarmManager.h"
#include "Storage.h"

AlarmManager alarmManager;

void AlarmManager::init(TimeManager *timeManager)
{
    _timeManager = timeManager;
}

void AlarmManager::processRelayString(const String& relayStr) {
    // Format: "1:1000,0:0,1:2000,1:3000"
    // relayStr = "1:1000,0:0,1:2000,1:3000"
    Serial.println("Processing relay string: " + relayStr);
    
    int index = 0;
    int relayIndex = 1; // Track which relay number we're processing
    
    while (index < relayStr.length()) {
        int nextComma = relayStr.indexOf(',', index);
        if (nextComma == -1) nextComma = relayStr.length();
        
        String relayPart = relayStr.substring(index, nextComma);
        int colonPos = relayPart.indexOf(':');
        
        if (colonPos != -1) {
            byte shouldTurnOn = relayPart.substring(0, colonPos).toInt();
            unsigned long duration = relayPart.substring(colonPos + 1).toInt();
            
            Serial.print("Relay ");
            Serial.print(relayIndex);
            Serial.print(" should be: ");
            Serial.print(shouldTurnOn ? "ON" : "OFF");
            Serial.print(" for ");
            Serial.print(duration);
            Serial.println("ms");
            
            if (shouldTurnOn == 1 && duration > 0) {
                relay.on(relayIndex);
                RelayTimer timer = {
                    .relayNumber = (byte)relayIndex,
                    .startTime = millis(),
                    .duration = duration
                };
                activeRelays.push_back(timer);
                
                Serial.print("Added timer for relay ");
                Serial.println(relayIndex);
            }
        }
        
        index = nextComma + 1;
        relayIndex++; // Increment to next relay number
    }
}

void AlarmManager::updateRelayTimers() {
    unsigned long currentTime = millis();
    
    for (auto it = activeRelays.begin(); it != activeRelays.end();) {
        if (currentTime - it->startTime >= it->duration) {
            Serial.print("Turning off relay ");
            Serial.print(it->relayNumber);
            Serial.println(" due to timeout");
            
            relay.off(it->relayNumber);
            it = activeRelays.erase(it);
        } else {
            ++it;
        }
    }
}

void AlarmManager::checkAlarms() {
    if (!_timeManager->isTimeSet())
        return;

    // Update any active relay timers
    updateRelayTimers();

    String alarmsJson = storage.readJSON("/alarm.json");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, alarmsJson);

    if (error)
        return;

    JsonArray alarms = doc.as<JsonArray>();

    int currentHour = _timeManager->getHours();
    int currentMinute = _timeManager->getMinutes();
    int currentDay = _timeManager->getDay();

    static bool alarmTriggeredThisMinute = false;

    if (_timeManager->getSeconds() > 0) {
        alarmTriggeredThisMinute = false;
    }

    if (!alarmTriggeredThisMinute) {
        for (JsonVariant alarmJson : alarms) {
            String timeStr = alarmJson["time"].as<String>();
            int alarmHour = timeStr.substring(0, 2).toInt();
            int alarmMinute = timeStr.substring(3).toInt();

            String daysStr = alarmJson["days"].as<String>();
            std::reverse(daysStr.begin(), daysStr.end());
            int days = strtol(daysStr.c_str(), nullptr, 2);

            bool isDayActive = (days & (1 << currentDay));

            if (currentHour == alarmHour &&
                currentMinute == alarmMinute &&
                _timeManager->getSeconds() == 0 &&
                isDayActive) {

                int ringtone = alarmJson["ringtone"].as<int>();
                String relayStr = alarmJson["relay"].as<String>();

                // Trigger alarm actions
                buzzer.beep(1);
                audio.play(ringtone);
                processRelayString(relayStr);

                Serial.println("========ALARM TRIGGERED=========");
                alarmTriggeredThisMinute = true;
                break;
            }
        }
    }
}

void AlarmManager::addAlarm(const String &label, const String &time, String days, int ringtone, const String &relay)
{
    String alarmsJson = storage.readJSON("/alarm.json");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, alarmsJson);

    JsonArray alarms;
    if (error || doc.isNull() || (doc.is<JsonObject>() && doc.size() == 0) || (doc.is<JsonArray>() && doc.size() == 0))
    {
        // If JSON is invalid or null, create a new array
        alarms = doc.to<JsonArray>();
    }
    else
    {
        alarms = doc.as<JsonArray>();
    }

    int nextId = 1;
    for (JsonVariant alarmJson : alarms)
    {
        int currentId = alarmJson["id"].as<int>();
        if (currentId >= nextId)
        {
            nextId = currentId + 1;
        }
    }

    JsonObject newAlarm = alarms.createNestedObject();
    newAlarm["id"] = nextId;
    newAlarm["label"] = label;
    newAlarm["time"] = time;
    newAlarm["days"] = days;
    newAlarm["ringtone"] = ringtone;
    newAlarm["relay"] = relay;

    String updatedJson;
    serializeJsonPretty(doc, updatedJson);
    storage.writeJSON("/alarm.json", updatedJson);
}

void AlarmManager::removeAlarm(int id)
{
    String alarmsJson = storage.readJSON("/alarm.json");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, alarmsJson);

    if (error)
    {
        Serial.println("Failed to parse alarm JSON");
        return;
    }

    JsonArray alarms = doc.as<JsonArray>();

    for (size_t i = 0; i < alarms.size(); i++)
    {
        if (alarms[i]["id"].as<int>() == id)
        {
            alarms.remove(i);
            break;
        }
    }

    int newId = 1;
    for (JsonVariant alarmJson : alarms)
    {
        alarmJson["id"] = newId++;
    }

    String updatedJson;
    serializeJsonPretty(doc, updatedJson);
    storage.writeJSON("/alarm.json", updatedJson);
}

bool AlarmManager::updateAlarm(int id, String label, String time, String days, int ringtone, String relay)
{
    String alarmsJson = storage.readJSON("/alarm.json");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, alarmsJson);

    if (error)
    {
        return false;
    }

    JsonArray alarms = doc.as<JsonArray>();

    for (JsonObject alarm : alarms)
    {
        if (alarm["id"] == id)
        {
            // Update the alarm properties
            alarm["label"] = label;
            alarm["time"] = time;
            alarm["days"] = days; // Store days as string to preserve leading zero
            alarm["ringtone"] = ringtone;
            alarm["relay"] = relay;

            // Save the updated alarms back to the file
            String updatedAlarmsJson;
            serializeJsonPretty(doc, updatedAlarmsJson);
            storage.writeJSON("/alarm.json", updatedAlarmsJson);

            return true;
        }
    }

    return false; // Alarm with the specified ID was not found
}

String AlarmManager::getAlarmsJson()
{
    return storage.readJSON("/alarm.json");
}