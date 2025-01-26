#include "Buzzer.h"
#include "Config.h"

Buzzer buzzer;

void Buzzer::init()
{
    pinMode(BUZZER_PIN, OUTPUT);
}

void Buzzer::beep(int toneType)
{
    switch (toneType)
    {
    case 1:
        tone(BUZZER_PIN, 784);
        delay(100);
        tone(BUZZER_PIN, 1047);
        delay(100);
        noTone(BUZZER_PIN);
        break;
    case 2:
        tone(BUZZER_PIN, 523);
        delay(100);
        tone(BUZZER_PIN, 392);
        delay(100);
        noTone(BUZZER_PIN);
        break;
    case 3:
        tone(BUZZER_PIN, 523);
        delay(100);
        tone(BUZZER_PIN, 784);
        delay(100);
        noTone(BUZZER_PIN);
        break;
    case 4:
        tone(BUZZER_PIN, 523);
        delay(100);
        tone(BUZZER_PIN, 784);
        delay(100);
        noTone(BUZZER_PIN);
        break;
    case 5:
        tone(BUZZER_PIN, 523);
        delay(100);
        tone(BUZZER_PIN, 784);
        delay(100);
        noTone(BUZZER_PIN);
        break;
    case 6:
        tone(BUZZER_PIN, 523);
        delay(100);
        tone(BUZZER_PIN, 784);
        delay(100);
        noTone(BUZZER_PIN);
        break;
    case 7:
        tone(BUZZER_PIN, 523);
        delay(100);
        tone(BUZZER_PIN, 784);
        delay(100);
        noTone(BUZZER_PIN);
        break;
    case 8:
        tone(BUZZER_PIN, 523);
        delay(100);
        tone(BUZZER_PIN, 784);
        delay(100);
        noTone(BUZZER_PIN);
        break;
    case 9:
        tone(BUZZER_PIN, 523);
        delay(100);
        tone(BUZZER_PIN, 784);
        delay(100);
        noTone(BUZZER_PIN);
        break;
    case 10:
        tone(BUZZER_PIN, 523);
        delay(100);
        tone(BUZZER_PIN, 784);
        delay(100);
        noTone(BUZZER_PIN);
        break;
    }
}