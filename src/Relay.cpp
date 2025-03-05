#include "Relay.h"
#include <Arduino.h>
#include "Config.h"
#include "Storage.h"
#include "Buzzer.h"

Relay relay;

int relayPins[4] = {RELAY1_PIN, RELAY2_PIN, RELAY3_PIN, RELAY4_PIN};

String Relay::getRelayState(byte relayNumber)
{
    String jsonContent = storage.readJSON("relay.json");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonContent);
    if (error)
    {
        Serial.println("Failed to parse JSON");
        return "off"; // Nilai default jika JSON rusak atau kosong
    }
    String relayKey = "relay" + String(relayNumber);
    if (doc[relayKey].is<int>())
    {
        return doc[relayKey].as<String>(); // Mengembalikan status dari key tersebut
    }
    else
    {
        return "off"; // Jika key tidak ditemukan, kembalikan status default "off"
    }
}

void Relay::init()
{
    String relay1State = getRelayState(1); // Membaca status relay1
    String relay2State = getRelayState(2); // Membaca status relay2
    String relay3State = getRelayState(3); // Membaca status relay2
    String relay4State = getRelayState(4); // Membaca status relay2

    // Mengonversi status string menjadi HIGH atau LOW
    int relay1StateValue = (relay1State == "on") ? RELAY_ON : RELAY_OFF;
    int relay2StateValue = (relay2State == "on") ? RELAY_ON : RELAY_OFF;
    int relay3StateValue = (relay3State == "on") ? RELAY_ON : RELAY_OFF;
    int relay4StateValue = (relay4State == "on") ? RELAY_ON : RELAY_OFF;

    pinMode(RELAY1_PIN, OUTPUT);
    pinMode(RELAY2_PIN, OUTPUT);
    pinMode(RELAY3_PIN, OUTPUT);
    pinMode(RELAY4_PIN, OUTPUT);

    digitalWrite(RELAY1_PIN, relay1StateValue);
    Serial.println("relay.cpp relay1 init: " + relay1State);
    digitalWrite(RELAY2_PIN, relay2StateValue);
    Serial.println("relay.cpp relay2 init: " + relay2State);
    digitalWrite(RELAY3_PIN, relay3StateValue);
    Serial.println("relay.cpp relay3 init: " + relay2State);
    digitalWrite(RELAY4_PIN, relay4StateValue);
    Serial.println("relay.cpp relay4 init: " + relay2State);
}

void Relay::on(byte relayNumber) {
    String relayKey = "relay" + String(relayNumber);
    if (relayNumber < 1 || relayNumber > 4) {
        Serial.println("Invalid relay number");
        buzzer.beep(2);
        return;
    }

    // Read existing JSON
    String jsonContent = storage.readJSON("relay.json");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonContent);
    if (error) {
        Serial.println("Failed to parse JSON");
        return;
    }

    // Update state in nested structure
    doc[relayKey]["state"] = "on";
    // Keep existing duration value
    if (!doc[relayKey]["duration"].is<int>()) {
        doc[relayKey]["duration"] = 1000; // Default duration if not set
    }

    // Convert to string and save
    String updatedJson;
    serializeJsonPretty(doc, updatedJson);
    storage.writeJSON("relay.json", updatedJson);

    // Set the pin
    byte pin = relayPins[relayNumber - 1];
    digitalWrite(pin, RELAY_ON);
    Serial.println("Relay" + String(relayNumber) + " turned ON");
}

void Relay::off(byte relayNumber) {
    String relayKey = "relay" + String(relayNumber);
    if (relayNumber < 1 || relayNumber > 4) {
        Serial.println("Invalid relay number");
        buzzer.beep(2);
        return;
    }

    // Read existing JSON
    String jsonContent = storage.readJSON("relay.json");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonContent);
    if (error) {
        Serial.println("Failed to parse JSON");
        return;
    }

    // Update state in nested structure
    doc[relayKey]["state"] = "off";
    // Keep existing duration value
    if (!doc[relayKey]["duration"].is<int>()) {
        doc[relayKey]["duration"] = 1000; // Default duration if not set
    }

    // Convert to string and save
    String updatedJson;
    serializeJsonPretty(doc, updatedJson);
    storage.writeJSON("relay.json", updatedJson);

    // Set the pin
    byte pin = relayPins[relayNumber - 1];
    digitalWrite(pin, RELAY_OFF);
    Serial.println("Relay" + String(relayNumber) + " turned OFF");
}

void Relay::toggle(byte relayNumber) {
    String relayKey = "relay" + String(relayNumber);
    if (relayNumber < 1 || relayNumber > 4) {
        Serial.println("Invalid relay number");
        buzzer.beep(2);
        return;
    }

    // Read existing JSON
    String jsonContent = storage.readJSON("relay.json");
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonContent);
    if (error) {
        Serial.println("Failed to parse JSON");
        return;
    }

    // Get current state and toggle it
    String currentState = doc[relayKey]["state"] | "off";
    String newState = (currentState == "on") ? "off" : "on";
    
    // Update state in nested structure
    doc[relayKey]["state"] = newState;
    // Keep existing duration value
    if (!doc[relayKey]["duration"].is<int>()) {
        doc[relayKey]["duration"] = 1000; // Default duration if not set
    }

    // Convert to string and save
    String updatedJson;
    serializeJsonPretty(doc, updatedJson);
    storage.writeJSON("relay.json", updatedJson);

    // Set the pin
    byte pin = relayPins[relayNumber - 1];
    digitalWrite(pin, (newState == "on") ? RELAY_ON : RELAY_OFF);
    Serial.println("Relay" + String(relayNumber) + " toggled to " + newState);
}

void Relay::onWithDuration(byte relayNumber, unsigned long duration) {
    if (relayNumber < 1 || relayNumber > 4) {
        Serial.println("Invalid relay number");
        buzzer.beep(2);
        return;
    }
    
    // Turn on the relay
    byte pin = relayPins[relayNumber - 1];
    digitalWrite(pin, RELAY_ON);
    
    // Add timer
    RelayTimer timer = {
        .relayNumber = relayNumber,
        .startTime = millis(),
        .duration = duration
    };
    activeTimers.push_back(timer);
    
    Serial.println("Relay" + String(relayNumber) + " turned ON for " + String(duration) + "ms");
}

void Relay::update() {
    unsigned long currentTime = millis();
    
    for (auto it = activeTimers.begin(); it != activeTimers.end();) {
        if (currentTime - it->startTime >= it->duration) {
            // Turn off the relay
            off(it->relayNumber);
            Serial.println("Relay" + String(it->relayNumber) + " duration completed");
            it = activeTimers.erase(it);
        } else {
            ++it;
        }
    }
}