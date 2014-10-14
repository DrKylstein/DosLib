#include <dos.h>
#include <conio.h>
#include "Timer.hpp"


#define TIMER_PULSE 0x0012305E
#define BIOS_DIVISOR 0x10000

using std::uint_fast64_t;
using std::uint16_t;

static Timer::interrupt_ptr Timer::_prev_int_8 = 0;
static void (__far* Timer::_task)() = 0;
static volatile Timer::tics_t Timer::_count = 0;
static volatile Timer::tics_t Timer::_lastBiosCount = 0;
static unsigned int Timer::_biosStep = 1;

Timer::Timer(int hz): _nextCount(0) {
    atomic lock;
    _hz = hz;
    _prev_int_8 = _dos_getvect(8);
    _dos_setvect(8, _timer_int);
    //set a more precise interval
    unsigned int divisor = TIMER_PULSE / _hz;
    _biosStep = BIOS_DIVISOR / divisor;
    outp(0x43, 0x36);
    outp(0x40, divisor & 0xFF);
    outp(0x40, divisor >> 8);
}
Timer::~Timer() {
    atomic lock;
    //reset interval, 0x0000 = 0x10000
    outp(0x43, 0x36);
    outp(0x40, 0);
    outp(0x40, 0);
    _dos_setvect(8, _prev_int_8);
}
static void __interrupt __far Timer::_timer_int() {
    _count++;
    if(_task != 0) {
        _task();
    }
    if(_count > _lastBiosCount + _biosStep) {
        _chain_intr(_prev_int_8);
        _lastBiosCount = _count;
    } else {
        //notify Programmable Interrupt Controller that we've finished
        outp(0x20, 0x20);
    }
}
void Timer::throttle(int fps) {
    tics_t newCount;
    do {
        atomic lock;
        newCount = _count;
    } while(newCount < _nextCount);
    
    long int period = _hz / fps;
    _nextCount = newCount + period;
}
void Timer::setTask(void (__far* task)()) {
    _task = task;
}

unsigned int Timer::second() {
    return _hz;
}
Timer::tics_t Timer::getTics() {
    atomic lock;
    return _count;
}
