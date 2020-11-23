#ifndef INTERRUPT_H
#define INTERRUPT_H

#include <iostream>

#include "mmu.h"
#include "cpu.h"
#include "gpu.h"

class Interrupter
{
    private:
        bool MasterInterrupt;
        MMU *mmu;
        Cpu *cpu;
        Gpu *gpu;
    public:
        Interrupter(MMU &Mmu, Cpu &m_cpu, Gpu &m_gpu);
        ~Interrupter();
        void handleInterrupts();
        void requestInterrupt(int id);
        void serviceInterrupt(int id);
};

#endif // INTERRUPT_H
