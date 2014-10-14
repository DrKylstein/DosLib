#ifndef INTERRUPTS_HPP
#define INTERRUPTS_HPP
namespace interrupts {
typedef void (__interrupt __far *isr_ptr)();
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
}
#endif