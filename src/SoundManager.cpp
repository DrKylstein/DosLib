#include <fstream>
#include <conio.h>
#include "SoundManager.hpp"
#include <iomanip>

static void SoundManager::_detectAdlib() {
    _adlibAddr = 0x388;
}
static void SoundManager::_resetAdlib() {
	for(int i =1; i < 0xF6; ++i) {
		_writeAdlib(i,0);
	}
    _writeAdlib(0x01,0x20);
    _writeAdlib(0x08,0x00);
}
static void SoundManager::_writeAdlib(uint8_t reg, uint8_t value) {
	outp(_adlibAddr, reg); //index write
	for(int i = 0; i < 6; ++i) { //delay loop
		inp(_adlibAddr);
	}
	outp(_adlibAddr+1, value); //data write
	for(int i = 0; i < 36; ++i) { //more delay
		inp(_adlibAddr);
	}
}
void static SoundManager::_stopSound() {
    _currentSound = -1;
    _lastPitch = 0;
    _soundPos = 0;
    _writeAdlib(0xB0, 0);
}
void SoundManager::stopSound() {
    _stopSound();
}

SoundTask SoundManager::getTask() {
    return &_update;
}

SoundManager::SoundManager() {
    _detectAdlib();
    _resetAdlib();
}
SoundManager::~SoundManager() {
    stopMusic();
    _stopSound();
    _resetAdlib();
    if(_songLength > 0) {
        _songLength = 0;
        delete[] _songData;
    }
    for(int i = _soundCount-1; i >= 0; i--) {
        _soundCount--;
        delete _sounds[i];
    }
    delete[] _sounds;
}
void SoundManager::playMusic(const char* filename) {
    stopMusic();
    if(_songLength > 0) {
        _songLength = 0;
        delete[] _songData;
    }
    
    std::ifstream file(filename, std::ios::binary);
    
    uint32_t begin = file.tellg();
    file.seekg(0, std::ios::end);
    uint32_t fileLen = file.tellg() - begin;
    file.seekg(2, std::ios::beg);
    
    _songLength = file.get();
    _songLength |= file.get() << 8;
    
    if(_songLength == 0) {
        _songLength = fileLen;
        file.seekg(0, std::ios::beg);
    }
    
    _songData = new SongData[_songLength/sizeof(SongData)];
    file.read((uint8_t*)_songData, _songLength);
        
    _songIndex = 0;
    _delay = 0;
    _musicPlaying = true;
}
static void SoundManager::_silenceMusic() {
    for(int i = 0xB1; i <= 0xB8; i++) {
        _writeAdlib(i, 0);
    }
}
void SoundManager::stopMusic() {
    _musicPlaying = false;
    _silenceMusic();
}
static void SoundManager::_updateAdlibSound() {
    uint8_t nextPitch = _sounds[_currentSound]->pitches[_soundPos];
    if(nextPitch != _lastPitch) {
        _writeAdlib(0xA0, nextPitch);
        if(nextPitch == 0) {
            _writeAdlib(0xB0, _sounds[_currentSound]->block);
        } else if(_lastPitch == 0) {
            _writeAdlib(0xB0, _sounds[_currentSound]->block | 0x20);
        }
    }
    _lastPitch = nextPitch;
    if(++_soundPos >= _sounds[_currentSound]->length) {
        _stopSound();
    }
}

static void SoundManager::_updateAdlibMusic() {
    if(_delay > 0) {
        _delay--;
        return;
    }
    if(_songIndex >= _songLength/sizeof(SongData)) {
        _songIndex = 0;
        _silenceMusic();
    }
    //std::cout << (int)_songData[_songIndex] << " = "  << (int)_songData[_songIndex+1] << std::endl;
    _writeAdlib(_songData[_songIndex].reg, _songData[_songIndex].value);
    _delay = _songData[_songIndex].delay;
    _songIndex++;
}

static void SoundManager::_update() {
    if(_soundTimer++ >= 3) {
        _soundTimer = 0;
            if(_currentSound > -1) {
                _updateAdlibSound();
            }
    }
    if(_musicPlaying) {
        _updateAdlibMusic();
    }
}

void SoundManager::loadSounds(const char* filename) {
    char buffer[32];
    std::ifstream file(filename);
    file >> _soundCount >> std::ws;
    
    _sounds = new AdlibSound*[_soundCount];
    for(int i = 0; i < _soundCount; i++) {
        file.getline(buffer, 32);
        _sounds[i] = new AdlibSound(buffer);
    }
}

void SoundManager::playSound(int index) {
    if(index >= _soundCount || index < -1) {
        return;
    }
    if(_currentSound > -1 && _sounds[_currentSound]->priority > _sounds[index]->priority) {
        return;
    }
    stopSound();
    for(int i = 0; i < 11; i++) {
        _writeAdlib(_soundRegs[i], _sounds[index]->instrument[i]);
    }
    _currentSound = index;
}


static SoundManager::SongData* SoundManager::_songData = 0;
static uint16_t SoundManager::_delay = 0;
static uint32_t SoundManager::_songLength = 0;
static uint32_t SoundManager::_songIndex = 0;
static uint16_t SoundManager::_adlibAddr = 0;

static int SoundManager::_soundTimer = 0;
static SoundManager::AdlibSound** SoundManager::_sounds = 0;
static int SoundManager::_soundCount = 0;
static volatile int SoundManager::_soundPos = 0;
static volatile int SoundManager::_currentSound = -1;
static volatile uint8_t SoundManager::_lastPitch = 0;
static const uint8_t SoundManager::_soundRegs[] = {
    0x20, 0x23, 0x40, 0x43, 0x60, 0x63, 0x80, 0x83, 0xE0, 0xE3, 0xC0
};
static bool SoundManager::_musicPlaying = false;


SoundManager::AdlibSound::AdlibSound(const char* filename) {
    int pitchStart = 0;
    int pitchEnd = 0;
    std::ifstream file(filename, std::ios::binary);
    file.read((char*)&length, 4);
    file.read((char*)&priority, 2);
    file.read(instrument, 16);
    block = (file.get() & 0x7) << 2;
    pitches = new uint8_t[length];
    file.read(pitches, length);
}
SoundManager::AdlibSound::~AdlibSound() {
    delete[] pitches;
}
