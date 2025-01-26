#ifndef AUDIO_H
#define AUDIO_H

#include <DFPlayerMini_Fast.h>
#include <SoftwareSerial.h>
#include "Config.h"

class Audio
{
public:
    // Inisialisasi audio
    void init();

    // Kontrol pemutaran
    void play(uint16_t trackNum);
    void pause();
    void resume();
    void playNext();
    void playPrevious();
    void stop();

    // Pemutaran dari folder
    void playFolder(uint8_t folderNum, uint8_t trackNum);

    // Iklan audio
    void playAdvertisement(uint16_t trackNum);
    void stopAdvertisement();

    // Kontrol volume
    void incVolume();
    void decVolume();
    void volume(uint8_t vol);

    // Kontrol equalizer
    void eq(uint8_t eq);

    // Mode pemutaran
    void loop(uint16_t trackNum); 
    void playbackSource(uint8_t source);

    // Mode daya
    void sleep();
    void wakeUp();
};


#endif // AUDIO_H

extern Audio audio;