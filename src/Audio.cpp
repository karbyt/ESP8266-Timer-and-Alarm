#include "Audio.h"

SoftwareSerial mySerial(RX_AUDIO_PIN, TX_AUDIO_PIN); 
DFPlayerMini_Fast mp3;
Audio audio;

void Audio::init()
{
    mySerial.begin(9600);
    mp3.begin(mySerial, false);
    mp3.volume(DEFAULT_VOLUME);
    mp3.EQSelect(DEFAULT_EQ);
}


void Audio::play(uint16_t trackNum)
{
    mp3.play(trackNum);
}

void Audio::pause()
{
    mp3.pause();
}

void Audio::resume()
{
    mp3.resume();
}

void Audio::playNext()
{
    mp3.playNext();
}

void Audio::playPrevious()
{
    mp3.playPrevious();
}

void Audio::stop()
{
    mp3.stop();
}

void Audio::playFolder(uint8_t folderNum, uint8_t trackNum)
{
    mp3.playFolder(folderNum, trackNum);
}

void Audio::playAdvertisement(uint16_t trackNum)
{
    mp3.playAdvertisement(trackNum);
}

void Audio::stopAdvertisement()
{
    mp3.stopAdvertisement();
}

void Audio::incVolume()
{
    mp3.incVolume();
}

void Audio::decVolume()
{
    mp3.decVolume();
}

void Audio::volume(uint8_t vol)
{
    mp3.volume(vol);
}

void Audio::eq(uint8_t eq)
{
    mp3.EQSelect(eq);
}

void Audio::loop(uint16_t trackNum)
{
    mp3.loop(trackNum);
}

void Audio::playbackSource(uint8_t source)
{
    mp3.playbackSource(source);
}

void Audio::sleep()
{
    mp3.sleep();
}

void Audio::wakeUp()
{
    mp3.wakeUp();
}
