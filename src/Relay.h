#ifndef RELAY_H
#define RELAY_H

#include <Arduino.h>

class Relay
{
public:
    void init();
    void on(byte relayNumber);
    void off(byte relayNumber);
    void toggle(byte relayNumber);
    void onWithDuration(byte relayNumber, unsigned long duration);
    void update();

private:
    String getRelayState(byte relayNumber);
    struct RelayTimer {
        byte relayNumber;
        unsigned long startTime;
        unsigned long duration;
    };
    std::vector<RelayTimer> activeTimers;
    void updateTimers();
};

#endif // RELAY_H

extern Relay relay;