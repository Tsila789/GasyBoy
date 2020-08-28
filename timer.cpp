#include "timer.h"

Timer::Timer()
{

}

Timer::~Timer()
{

}

Timer::Timer(MMU &mmu)
{
    Mmu = &mmu;
    divide_counter = 0;
    timer_counter = 1024;
}

void Timer::update_timer(int m_cycle)
{
	divider(m_cycle);
	if (timer_on())
	{
		timer_counter -= m_cycle;
		if (timer_counter <= 0)
		{
			setFreq();
			uint8_t val = Mmu->read_ram(0xFF05);
			if (val == 0xFF)
			{
			    Mmu->write_ram(0xFF05, Mmu->read_ram(0xFF06));
				requestInterrupt(2);
			}
			else
                Mmu->write_ram(0xFF05, val + 1);
		}
	}
}

int Timer::timer_on()
{
	uint8_t control = Mmu->read_ram(0xFF07);
	return (control >> 2) & 1;
}

void Timer::setFreq()
{
	uint8_t	freq = Mmu->read_ram(0xFF07) & 0x3;
	if (freq == 0x0)
		timer_counter = 1024;
	else if (freq == 0x1)
		timer_counter = 16;
	else if (freq == 0x2)
		timer_counter = 64;
	else if (freq == 0x3)
		timer_counter = 256;
}

void Timer::divider(int m_cycle)
{
	divide_counter += m_cycle;
	if (divide_counter >= 255)
	{
		divide_counter = 0;
		Mmu->direct_set(0xFF04, Mmu->read_ram(0xFF04) + 1);
	}
}

void Timer::requestInterrupt(int id)
{
    uint8_t req = Mmu->read_ram(0xFF0F);
    req |= (1 << id);
    Mmu->write_ram(0xFF0F, req);
}
