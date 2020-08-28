#ifndef TIMER_H
#define TIMER_H

#include <iostream>
#include "mmu.h"

class Timer
{
    private:
        MMU *Mmu;
        int divide_counter, timer_counter;
    public:
        Timer();
        Timer(MMU &mmu);
        ~Timer();
        int timer_on();
        void setFreq();
        void divider(int m_cycle);
        void update_timer(int m_cycle);
        void change_interrupt(bool value);
        bool get_interrupt_status();
        void requestInterrupt(int id);
};

#endif // TIMER_H
