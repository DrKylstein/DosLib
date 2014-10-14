#ifndef TIMER_HPP
#define TIMER_HPP

#include <cstdint>
class Timer {
    public:
        typedef std::uint_fast64_t tics_t;   
    
        Timer(int hz);
        ~Timer();
        void throttle(int fps);
        void setTask(void (__far* task)());
        tics_t getTics();       
        unsigned int second();
        
    private:
        class atomic {
            public:
                inline atomic() {
                    _asm {
                        cli
                    }
                }
                inline ~atomic() {
                    _asm {
                        sti
                    }
                }
        };
        typedef void (__interrupt __far *interrupt_ptr)();
        
        static interrupt_ptr _prev_int_8;
        static void __interrupt __far _timer_int();
        static void (__far* _task)();
        static volatile tics_t _count;
        static volatile tics_t _lastBiosCount;
        static unsigned int _biosStep;
        unsigned int _hz; 
        tics_t _nextCount;
        std::uint16_t _oldInterval;
};
#endif