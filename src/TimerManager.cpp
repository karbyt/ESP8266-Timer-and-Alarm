#include "TimerManager.h"
#include "Config.h"

TimerManager timerManager;

TimerManager::TimerManager() : isRunning(false), baseTime(0)
{
    relayPins[0] = RELAY1_PIN;
    relayPins[1] = RELAY2_PIN;
    relayPins[2] = RELAY3_PIN;
    relayPins[3] = RELAY4_PIN;
}

void TimerManager::saveRunningState()
{
    String stateJson = "{\"isRunning\":" + String(isRunning ? "true" : "false") + "}";
    storage.writeJSON("/timerstate.json", stateJson);
}

bool TimerManager::loadRunningState()
{
    String stateJson = storage.readJSON("/timerstate.json");
    if (stateJson == "{}")
    {
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, stateJson);
    if (error)
    {
        Serial.println("Failed to parse timerstate.json");
        return false;
    }

    return doc["isRunning"] | false;
}

void TimerManager::saveConfig()
{
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();

    for (int i = 0; i < 4; i++)
    {
        if (relayConfigs[i].cycle != 0 || relayConfigs[i].cycle == -1)
        {
            JsonObject timer = array.add<JsonObject>();
            timer["id"] = relayConfigs[i].id;
            timer["on_time"] = relayConfigs[i].on_time;
            timer["off_time"] = relayConfigs[i].off_time;
            timer["cycle"] = relayConfigs[i].initial_cycle;
            timer["reversed"] = relayConfigs[i].reversed;
        }
    }

    String configJson;
    serializeJsonPretty(doc, configJson);
    storage.writeJSON("/timer.json", configJson);
}

void TimerManager::loadConfig()
{
    String configJson = storage.readJSON("/timer.json");

    // Reset all configs
    for (int i = 0; i < 4; i++)
    {
        relayConfigs[i].id = i + 1;
        relayConfigs[i].on_time = 0;
        relayConfigs[i].off_time = 0;
        relayConfigs[i].cycle = 0;
        relayConfigs[i].initial_cycle = 0;
        relayConfigs[i].reversed = false;
        relayState[i] = RELAY_OFF;
        currentCycle[i] = 0;
        relayConfigs[i].startTime = 0;  // Reset startTime
        digitalWrite(relayPins[i], RELAY_OFF);
    }

    if (configJson == "{}")
    {
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, configJson);
    if (error)
    {
        Serial.println("Failed to parse timer.json");
        return;
    }

    for (JsonVariant v : doc.as<JsonArray>())
    {
        int id = v["id"].as<int>() - 1;
        if (id >= 0 && id < 4)
        {
            relayConfigs[id].id = v["id"].as<int>();
            relayConfigs[id].on_time = v["on_time"].as<unsigned long>();
            relayConfigs[id].off_time = v["off_time"].as<unsigned long>();
            relayConfigs[id].cycle = v["cycle"].as<int>();
            relayConfigs[id].initial_cycle = v["cycle"].as<int>();
            relayConfigs[id].reversed = v["reversed"].as<bool>();
        }
    }
}

void TimerManager::initializeRelayStates()
{
    unsigned long currentTime = millis();
    for (int i = 0; i < 4; i++)
    {
        // Only initialize if there's an active timer AND valid times
        if ((relayConfigs[i].cycle > 0 || relayConfigs[i].cycle == -1) && 
            (relayConfigs[i].on_time > 0 || relayConfigs[i].off_time > 0))
        {
            if (relayConfigs[i].reversed)
            {
                relayState[i] = RELAY_OFF;
                digitalWrite(relayPins[i], RELAY_OFF);
            }
            else
            {
                relayState[i] = RELAY_ON;
                digitalWrite(relayPins[i], RELAY_ON);
            }
            relayConfigs[i].startTime = currentTime;
        }
        else
        {
            // No active timer or invalid configuration
            relayState[i] = RELAY_OFF;
            digitalWrite(relayPins[i], RELAY_OFF);
        }
    }
}

void TimerManager::resetConfig()
{
    unsigned long currentTime = millis();
    for (int i = 0; i < 4; i++)
    {
        relayConfigs[i].cycle = relayConfigs[i].initial_cycle;
        // Hanya aktifkan relay jika timer memiliki konfigurasi valid
        if ((relayConfigs[i].cycle > 0 || relayConfigs[i].cycle == -1) && 
            (relayConfigs[i].on_time > 0 || relayConfigs[i].off_time > 0)) {
            relayState[i] = relayConfigs[i].reversed ? RELAY_OFF : RELAY_ON;
            digitalWrite(relayPins[i], relayState[i]);
        } else {
            // Pastikan relay mati jika tidak ada konfigurasi
            relayState[i] = RELAY_OFF;
            digitalWrite(relayPins[i], RELAY_OFF);
        }
        currentCycle[i] = 0;
        relayConfigs[i].startTime = currentTime;
    }
    Serial.println("Config reset!");
}

void TimerManager::resetCycles()
{
    for (int i = 0; i < 4; i++)
    {
        currentCycle[i] = 0;
        relayState[i] = relayConfigs[i].reversed ? RELAY_ON : RELAY_OFF;
        relayConfigs[i].startTime = millis();
        digitalWrite(relayPins[i], RELAY_OFF);
    }
}

void TimerManager::updateRelay(int index)
{
    if (relayConfigs[index].cycle == 0) return;

    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - relayConfigs[index].startTime;
    unsigned long cycleDuration = relayConfigs[index].on_time + relayConfigs[index].off_time;
    unsigned long timeInCycle = elapsedTime % cycleDuration;
    bool shouldBeOn = timeInCycle < relayConfigs[index].on_time;
    
    if (relayConfigs[index].reversed) shouldBeOn = !shouldBeOn;

    if (relayState[index] != shouldBeOn) {
        relayState[index] = shouldBeOn;
        // Langsung kontrol pin tanpa menggunakan class Relay
        digitalWrite(relayPins[index], shouldBeOn ? RELAY_ON : RELAY_OFF);
        
        if (!shouldBeOn && relayConfigs[index].cycle > 0) {
            currentCycle[index]++;
            if (currentCycle[index] >= relayConfigs[index].cycle) {
                relayConfigs[index].cycle = 0;
                digitalWrite(relayPins[index], RELAY_OFF);
                
                bool allFinished = true;
                bool hasInfinite = false;
                
                for (int i = 0; i < 4; i++) {
                    if (relayConfigs[i].cycle > 0) {
                        allFinished = false;
                        break;
                    } else if (relayConfigs[i].cycle == -1) {
                        hasInfinite = true;
                    }
                }
                
                if (allFinished && !hasInfinite) {
                    stop();
                }
            }
        }
    }
}

bool TimerManager::updateTimer(int id, unsigned long onTime, unsigned long offTime, int cycleCount, bool reversed)
{
    if (id < 1 || id > 4)
    {
        Serial.println("Invalid timer ID");
        return false;
    }

    int index = id - 1;

    relayConfigs[index].on_time = onTime;
    relayConfigs[index].off_time = offTime;
    relayConfigs[index].cycle = cycleCount;
    relayConfigs[index].initial_cycle = cycleCount;
    relayConfigs[index].reversed = reversed;

    saveConfig();
    return true;
}

void TimerManager::init()
{
    // Setup pins
    for (int i = 0; i < 4; i++)
    {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], RELAY_OFF);
    }

    // Load configuration
    loadConfig();

    // Load previous running state
    isRunning = loadRunningState();
    if (isRunning)
    {
        resetConfig();
        initializeRelayStates();
        Serial.println("Restored running state!");
    }
    else
    {
        stop();
    }
}

void TimerManager::update()
{
    if (isRunning)
    {
        for (int i = 0; i < 4; i++)
        {
            if (relayConfigs[i].cycle != 0 || relayConfigs[i].cycle == -1)
            {
                updateRelay(i);
            }
        }
    }
}

void TimerManager::start()
{
    resetConfig();
    baseTime = millis();  // Set waktu referensi untuk semua timer
    
    // Inisialisasi waktu mulai untuk setiap relay
    for (int i = 0; i < 4; i++) {
        relayConfigs[i].startTime = baseTime;
    }
    
    isRunning = true;
    saveRunningState();
    initializeRelayStates();
    Serial.println("Timer started!");
}

void TimerManager::stop()
{
    isRunning = false;
    saveRunningState();
    resetCycles();
    Serial.println("Timer stopped!");
}

bool TimerManager::addTimer(int id, unsigned long onTime, unsigned long offTime, int cycleCount, bool reversed)
{
    // Validasi id (1-4)
    if (id < 1 || id > 4) {
        Serial.println("Invalid timer ID (must be 1-4)");
        return false;
    }

    // Convert id to array index (0-3)
    int index = id - 1;

    // Cek apakah relay sudah digunakan
    if (relayConfigs[index].cycle != 0) {
        Serial.println("Timer ID " + String(id) + " is already in use");
        return false;
    }

    relayConfigs[index].id = id;
    relayConfigs[index].on_time = onTime;
    relayConfigs[index].off_time = offTime;
    relayConfigs[index].cycle = cycleCount;
    relayConfigs[index].initial_cycle = cycleCount;
    relayConfigs[index].reversed = reversed;

    saveConfig();
    Serial.println("Timer added to relay " + String(id));
    return true;
}

bool TimerManager::deleteTimer(int id)
{
    if (id < 1 || id > 4)
    {
        Serial.println("Invalid timer ID");
        return false;
    }

    int index = id - 1;

    relayConfigs[index].on_time = 0;
    relayConfigs[index].off_time = 0;
    relayConfigs[index].cycle = 0;
    relayConfigs[index].initial_cycle = 0;
    relayConfigs[index].reversed = false;
    digitalWrite(relayPins[index], RELAY_OFF);

    saveConfig();
    return true;
}