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
    if (doc.containsKey(relayKey))
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

    // Mengonversi status string menjadi HIGH atau LOW
    int relay1StateValue = (relay1State == "on") ? LOW : HIGH;
    int relay2StateValue = (relay2State == "on") ? LOW : HIGH;

    pinMode(RELAY1_PIN, OUTPUT);
    pinMode(RELAY2_PIN, OUTPUT);

    digitalWrite(RELAY1_PIN, relay1StateValue);
    Serial.println("relay1 init: " + relay1State);
    digitalWrite(RELAY2_PIN, relay2StateValue);
    Serial.println("relay2 init: " + relay2State);
}

void Relay::on(byte relayNumber)
{
    String relayKey = "relay" + String(relayNumber);
    if (relayNumber < 1 || relayNumber > 4)
    {
        Serial.println("Invalid relay number");
        buzzer.beep(2);
        return;
    }
    byte pin = relayPins[relayNumber - 1];
    digitalWrite(pin, RELAY_ON);
    buzzer.beep(1);
    storage.updateJSON("relay.json", relayKey, "on");
    Serial.println("Relay" + String(relayNumber) + " turned ON");
}

void Relay::off(byte relayNumber)
{
    String relayKey = "relay" + String(relayNumber);
    if (relayNumber < 1 || relayNumber > 4)
    {
        Serial.println("Invalid relay number");
        buzzer.beep(2);
        return;
    }
    byte pin = relayPins[relayNumber - 1];
    digitalWrite(pin, RELAY_OFF);
    buzzer.beep(1);
    storage.updateJSON("relay.json", relayKey, "off");
    Serial.println("Relay" + String(relayNumber) + " turned OFF");
}

void Relay::toggle(byte relayNumber)
{
    String relayKey = "relay" + String(relayNumber);
    if (relayNumber < 1 || relayNumber > 4)
    {
        Serial.println("Invalid relay number");
        buzzer.beep(2);
        return;
    }
    String currentState = getRelayState(relayNumber);
    String newState = (currentState == "on") ? "off" : "on";
    byte pin = relayPins[relayNumber - 1];
    digitalWrite(pin, (newState == "on") ? RELAY_ON : RELAY_OFF);
    buzzer.beep(1);
    storage.updateJSON("relay.json", relayKey, newState);
}
