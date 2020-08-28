#include "register.h"

Register::Register()
{
    regist = 0x0;
}

void Register::set(uint16_t hex)
{
    regist = hex;
}

uint16_t Register::reg()
{
    return regist;
}

uint8_t Register::lo()
{
    return (uint8_t)regist;
}

uint8_t Register::hi()
{
    return (uint8_t)(regist >> 8);
}

void Register::set_hi(uint8_t byte)
{
    regist = ((byte << 8) | (regist & 0x00FF));
}

void Register::set_lo(uint8_t byte)
{
    regist = ((regist & 0xFF00) | byte);
}

void Register::show()
{
    std::cout << std::hex << regist;
}

SpecialRegister::SpecialRegister() : Register()
{

}

bool SpecialRegister::get_zflag()
{
	return (lo() & (1<<7));
}

void SpecialRegister::set_zflag()
{
    uint8_t tmp = lo();
    tmp |= 1UL << 7;
    set_lo(tmp);
}

void SpecialRegister::clear_zflag()
{
    uint8_t tmp = lo();
    tmp &= ~(1UL << 7);
    set_lo(tmp);
}

bool SpecialRegister::get_subflag()
{
	return (lo() & (1<<6));
}

void SpecialRegister::set_subflag()
{
    uint8_t tmp = lo();
    tmp |= 1UL << 6;
    set_lo(tmp);
}

void SpecialRegister::clear_subflag()
{
    uint8_t tmp = lo();
    tmp &= ~(1UL << 6);
    set_lo(tmp);
}

bool SpecialRegister::get_halfcarryflag()
{
	return (lo() & (1<<5));
}

void SpecialRegister::set_halfcarryflag()
{
    uint8_t tmp = lo();
    tmp |= 1UL << 5;
    set_lo(tmp);
}

void SpecialRegister::clear_halfcarryflag()
{
    uint8_t tmp = lo();
    tmp &= ~(1UL << 5);
    set_lo(tmp);
}

bool SpecialRegister::get_carryflag()
{
	return (lo() & (1<<4));
}

void SpecialRegister::set_carryflag()
{
    uint8_t tmp = lo();
    tmp |= 1UL << 4;
    set_lo(tmp);
}

void SpecialRegister::clear_carryflag()
{
    uint8_t tmp = lo();
    tmp &= ~(1UL << 4);
    set_lo(tmp);
}

/*uint8_t SpecialRegister::lo()
{
    return (uint8_t)regist;
}

uint8_t SpecialRegister::hi()
{
    return (uint8_t)(regist >> 8);
}*/

