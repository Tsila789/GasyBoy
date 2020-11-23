#include "interrupt.h"

Interrupter::Interrupter(MMU &Mmu,Cpu &m_cpu, Gpu &m_gpu)
{
    MasterInterrupt = false;
    mmu = &Mmu;
    cpu = &m_cpu;
    gpu = &m_gpu;
}

Interrupter::~Interrupter()
{

}

void Interrupter::handleInterrupts()
{
    if (MasterInterrupt)
    {
        uint8_t req = mmu->read_ram(0xFF0F);
        uint8_t enable = mmu->read_ram(0xFFFF);
        if (req > 0)
        {
            for (int i=0; i<5; i++)
            {
                if (req & (1 << i))
                    if (enable & (1 << i))
                        serviceInterrupt(i);
            }
        }
    }
}

void Interrupter::requestInterrupt(int id)
{
    uint8_t req = mmu->read_ram(0xFF0F);
    req |= (1 << id);
    mmu->write_ram(0xFF0F, req);
}

void Interrupter::serviceInterrupt(int interrupt)
{
    MasterInterrupt = false;
    uint8_t req = mmu->read_ram(0xFF0F);
    req &= ~(1 << interrupt);
    mmu->write_ram(0xFF0F, req);
    cpu->push_SP(cpu->get_PC());
    switch (interrupt)
    {
        case 0: cpu->set_PC(0x40); break;
        case 1: cpu->set_PC(0x48); break;
        case 2: cpu->set_PC(0x50); break;
        case 4: cpu->set_PC(0x60); break;
    }
}

