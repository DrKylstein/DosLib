#ifndef SOUNDMANAGER_HPP
#define SOUNDMANAGER_HPP
#include <cstdint>
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
typedef void (__far* SoundTask)();
class SoundManager {
    public:
        SoundManager();
        ~SoundManager();
        SoundTask getTask();
        void playMusic(const char* filename);
        void stopMusic();
    
        void loadSounds(const char* filename);
        void playSound(int);
        void stopSound();
    
    private:
        
        struct SongData {
            uint8_t reg;
            uint8_t value;
            uint16_t delay;
        };
        struct AdlibSound {
            uint32_t length;
            uint16_t priority;
            uint8_t instrument[16];
            uint8_t block;
            uint8_t* pitches;
            
            AdlibSound(const char * filename);
            ~AdlibSound();
        };

    
        static uint16_t _adlibAddr;
    
        static SongData* _songData;
        static uint32_t _songLength;
        static uint32_t _songIndex;
        static uint16_t _delay;
    
        static int _soundTimer;
        static AdlibSound** _sounds;
        static int _soundCount;
        static volatile int _soundPos;
        static volatile int _currentSound;
        static volatile uint8_t _lastPitch;
    
        static void _update();
        static void _detectAdlib();
        static void _resetAdlib();
        static void _writeAdlib(uint8_t reg, uint8_t value);
        static void _stopSound();
        static void _silenceMusic();
        static void _updateAdlibSound();
        static void _updateAdlibMusic();
        
        static bool _musicPlaying;
    
        static const uint8_t _soundRegs[11];
};

#endif SOUNDMANAGER_HPP
