#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

class Buzzer
{
public:
    void init();
    void beep(int toneType);
};

extern Buzzer buzzer;

#endif