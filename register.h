#ifndef REG_H
#define REG_H

#include <iostream>

class Register
{
    protected:
        uint16_t regist;
    public:
        Register();
        uint16_t reg();
        uint8_t lo();
        uint8_t hi();
        void set(uint16_t hex);
        void set_lo(uint8_t byte);
        void set_hi(uint8_t byte);
        void show();
};

class SpecialRegister : public Register
{
	public:
	       SpecialRegister();
	       bool get_zflag();
	       void set_zflag();
           void clear_zflag();
           bool get_carryflag();
           void set_carryflag();
           void clear_carryflag();
           bool get_subflag();
	       void set_subflag();
           void clear_subflag();
           bool get_halfcarryflag();
	       void set_halfcarryflag();
           void clear_halfcarryflag();
           /*uint8_t lo();
           uint8_t hi();*/
};

#endif // REG_H
