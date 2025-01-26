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

private:
    String getRelayState(byte relayNumber);
};

#endif // RELAY_H

extern Relay relay;