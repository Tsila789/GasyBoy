#include "cpu.h"


void Cpu::fetchOpcode()
{
    currentOpcode = Mmu->read_ram(PC);
}

Cpu::Cpu()
{
    enable_interrupt = false;
}

Cpu::Cpu(MMU &mmu)
{
    enable_interrupt = false;
    Mmu = &mmu;
    AF.set(0x0);
    BC.set(0x0);
    DE.set(0x0);
    HL.set(0x0);
    PC = 0x0;
    prev_CP = 0x0;
    for (int i = 0x0; i < 0xFF; i++)
        SP[i] = 0x0;
    SP_indice = 0;
    cycle = 0;
    divide_counter = 0;
    timer_counter = 1024;
    start_debug = false;
    gpu_debug = false;
}

Cpu::~Cpu()
{

}

int Cpu::cpuStep()
{
    if ((Mmu->read_ram(0xFF50) == 0x01 && Mmu->get_boot_status())
        || (PC == 0x100 && Mmu->get_boot_status()))
        Mmu->disable_bios();
    prev_CP = PC;
    executeOpcode();
    if (start_debug)
        debug();
    return cycle;
}

int Cpu::get_cycle()
{
    return cycle;
}

uint16_t Cpu::get_next_2byte()
{
    return (Mmu->read_ram(PC+2) << 8) | Mmu->read_ram(PC+1);
}

uint16_t Cpu::get_next_2byte(uint16_t value)
{
    return (Mmu->read_ram(value+2) << 8) | Mmu->read_ram(value+1);
}

void Cpu::XOR_A(uint8_t reg)
{
    AF.set_hi(AF.hi() ^ AF.hi());
    if (AF.hi() == 0x0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.clear_carryflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::OR_A(uint8_t value)
{
    AF.set_hi(AF.hi() | value);
    if (AF.hi() == 0x0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.clear_carryflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::BIT(uint8_t byte, int i)
{
    if (!(byte & (1<<i)))
        AF.set_zflag();
    else if ((byte & (1<<i)))
        AF.clear_zflag();
    AF.clear_subflag();
    AF.set_halfcarryflag();
}

void Cpu::INC_Mem(uint16_t adrr)
{
    uint8_t tmp = Mmu->read_ram(adrr);
    Mmu->write_ram(adrr, tmp + 1);
    if (Mmu->read_ram(adrr) == 0)
    {
        AF.set_zflag();
        AF.set_halfcarryflag();
    }
    else
    {
        AF.clear_zflag();
        if ((tmp & 0x0F) + 1 > 0xFF)
            AF.set_halfcarryflag();
        else AF.clear_halfcarryflag();
    }
    AF.clear_subflag();
}

void Cpu::DEC_Mem(uint16_t adrr)
{
    uint8_t tmp = Mmu->read_ram(adrr);
    if ((tmp & 0x0F) - 0x1 < 0x00)
        AF.set_halfcarryflag();
    else AF.clear_halfcarryflag();
    Mmu->write_ram(adrr, tmp - 1);
    if (Mmu->read_ram(adrr) == 0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.set_subflag();
}

void Cpu::INC_reg_lo(Register &reg)
{
    reg.set_lo(reg.lo()+1);
    if (reg.lo() == 0)
    {
        AF.set_zflag();
        AF.set_halfcarryflag();
    }
    else
    {
        AF.clear_zflag();
        AF.clear_halfcarryflag();
    }
    AF.set_subflag();
}

void Cpu::INC_reg_hi(Register &reg)
{
    reg.set_hi(reg.hi()+1);
    if (reg.hi() == 0)
    {
        AF.set_zflag();
        AF.set_halfcarryflag();
    }
    else
    {
        AF.clear_zflag();
        AF.clear_halfcarryflag();
    }
    AF.clear_subflag();
}

void Cpu::INC_reg(Register &Reg)
{
    Reg.set(Reg.reg() + 1);
}

void Cpu::DEC_reg_hi(Register &reg)
{
    if ((reg.hi() & 0x0F) - 1 < 0)
        AF.set_halfcarryflag();
    else
        AF.clear_halfcarryflag();
    reg.set_hi(reg.hi()-1);
    if (reg.hi() == 0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.set_subflag();
}
void Cpu::DEC_reg_lo(Register &reg)
{
    if ((reg.lo() & 0x0F) - 1 < 0)
        AF.set_halfcarryflag();
    else
        AF.clear_halfcarryflag();
    reg.set_lo(reg.lo()-1);
    if (reg.lo() == 0x0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.set_subflag();
}

void Cpu::DEC_reg(Register &Reg)
{
    Reg.set(Reg.reg() - 1);
}


void Cpu::executeOpcode()
{
    fetchOpcode();
    switch (currentOpcode)
    {
         case 0x0: //NOP
            PC++;
            cycle = 4;
            break;
        case 0x01: //LD BC, d16
            LD_reg(BC, get_next_2byte());
            cycle = 12;
            PC += 3;
            break;
        case 0x02:
            LD_Mem(BC.reg(),AF.hi());
            cycle = 8;
            PC++;
            break;
        case 0x03:
            INC_reg(BC);
            cycle = 8;
            PC++;
            break;
        case 0x04:
            INC_reg_hi(BC);
            cycle = 4;
            PC++;
            break;
        case 0x05:
            DEC_reg_hi(BC);
            cycle = 4;
            PC++;
            break;
        case 0x06:
            LD_reg_hi(BC, Mmu->read_ram(PC+1));
            cycle = 8;
            PC+=2;
            break;
        case 0x07:
            RLCA();
            PC++;
            cycle = 4;
        break;
        case 0x08:
            push_SP(Mmu->read_ram(get_next_2byte()));
            PC += 3;
            cycle = 20;
            break;
        case 0x09:
            ADD_HL(BC.reg());
            cycle = 8;
            PC++;
            break;
        case 0x0A:
            LD_A(Mmu->read_ram(BC.reg()));
            cycle = 8;
            PC++;
            break;
        case 0x0B:
            DEC_reg(BC);
            cycle = 8;
            PC++;
            break;
        case 0x0C:
            INC_reg_lo(BC);
            cycle = 4;
            PC++;
            break;
        case 0x0D:
            DEC_reg_lo(BC);
            cycle = 4;
            PC++;
            break;
        case 0x0E:
            BC.set_lo(Mmu->read_ram(PC+1));
            cycle = 8;
            PC += 2;
            break;
        case 0x0F:
            RRCA();
            cycle = 4;
            PC++;
            break;
//        case 0x10:
//            //STOP 0 UNKNOWN
//            PC++;
//            cycle = 4;
//            break;
        case 0x11:
            DE.set(get_next_2byte());
            cycle = 12;
            PC += 3;
            break;
        case 0x12:
            LD_Mem(DE.reg(), AF.hi());
            cycle = 8;
            PC++;
            break;
        case 0x13:
            DE.set(DE.reg() + 1);
            cycle = 8;
            PC++;
            break;
        case 0x14:
            INC_reg_hi(DE);
            cycle = 4;
            PC++;
            break;
        case 0x15:
            DEC_reg_hi(DE);
            cycle = 4;
            PC++;
            break;
        case 0x16:
            DE.set_hi(Mmu->read_ram(PC+1));
            PC += 2;
            cycle = 8;
            break;
        case 0x17:
            RL_A();
            PC++;
            cycle = 4;
            break;
        case 0x18:
            PC += (int8_t)Mmu->read_ram(PC+1) + 2;
            cycle = 12;
            break;
        case 0x19:
            ADD_HL(DE.reg());
            cycle = 8;
            PC++;
            break;
        case 0x1A:
            AF.set_hi(Mmu->read_ram(DE.reg()));
            cycle = 8;
            PC++;
            break;
        case 0x1B:
            DEC_reg(DE);
            cycle = 8;
            PC++;
            break;
        case 0x1C:
            INC_reg_lo(DE);
            cycle = 4;
            PC++;
            break;
        case 0x1D:
            DEC_reg_lo(DE);
            cycle = 4;
            PC++;
            break;
        case 0x1E:
            DE.set_lo(Mmu->read_ram(PC+1));
            PC += 2;
            cycle = 8;
            break;
        case 0x1F:
            RR_reg_hi(AF);
            cycle = 4;
            PC++;
            break;
        case 0x20:
        {
            if (AF.get_zflag())
                cycle = 12;
            else
            {
                PC += (int8_t)Mmu->read_ram(PC+1);
                cycle = 16;
            }
            PC += 2;
            break;
        }
        case 0x21:
            HL.set(get_next_2byte());
            PC += 3;
            cycle = 12;
            break;
        case 0x22:
            Mmu->write_ram(HL.reg(), AF.hi());
            HL.set(HL.reg() + 1);
            cycle = 8;
            PC++;
            break;
        case 0x23:
            HL.set(HL.reg() + 1);
            cycle = 8;
            PC++;
            break;
        case 0x24:
            INC_reg_hi(HL);
            cycle = 4;
            PC++;
            break;
        case 0x25:
            DEC_reg_hi(HL);
            cycle = 4;
            PC++;
            break;
        case 0x26:
            LD_reg_hi(HL, Mmu->read_ram(PC+1));
            cycle = 8;
            PC+=2;
            break;
        case 0x27:
            //DECIMAL ADJUST TODO
            PC++;
            cycle = 4;
            break;
        case 0x28:
            if (AF.get_zflag())
                PC += (int8_t)Mmu->read_ram(PC+1);
            PC += 2;
            cycle = 12;
            break;
        case 0x29:
            ADD_HL(HL.reg());
            PC++;
            cycle = 8;
            break;
        case 0x2A:
            LD_A(Mmu->read_ram(HL.reg()));
            INC_reg(HL);
            cycle = 8;
            PC++;
            break;
        case 0x2B:
            DEC_reg(HL);
            cycle = 8;
            PC++;
            break;
        case 0x2C:
            INC_reg_lo(HL);
            cycle = 4;
            PC++;
            break;
        case 0x2D:
            DEC_reg_lo(HL);
            cycle = 4;
            PC++;
            break;
        case 0x2E:
            HL.set_lo(Mmu->read_ram(PC+1));
            PC += 2;
            cycle = 8;
            break;
        case 0x2F:
            CPL();
            PC++;
            cycle = 4;
            break;
        case 0x30:
            JR_NC(Mmu->read_ram(PC+1));
            cycle = 12;
            break;
        case 0x31:
            push_SP(get_next_2byte());
            PC += 3;
            cycle = 12;
            break;
        case 0x32:
            Mmu->write_ram(HL.reg(), AF.hi());
            HL.set(HL.reg()-1);
            cycle = 8;
            PC++;
            break;
        case 0x33:
            //NOT SURE
            SP[SP_indice]++;
            cycle = 8;
            PC++;
            break;
        case 0x34:
            INC_Mem(HL.reg());
            cycle = 12;
            PC++;
            break;
        case 0x35:
            DEC_Mem(HL.reg());
            cycle = 12;
            PC++;
            break;
        case 0x36:
            LD_Mem(HL.reg(), Mmu->read_ram(PC+1));
            PC+=2;
            cycle = 12;
            break;
        case 0x37:
            SCF();
            cycle = 4;
            PC++;
            break;
        case 0x38:
            JR_C(Mmu->read_ram(PC+1));
            cycle = 12;
            break;
        case 0x39:
            ADD_HL(SP[SP_indice]);
            cycle = 8;
            PC++;
            break;
        case 0x3A:
            LD_A(Mmu->read_ram(HL.reg()));
            DEC_reg(HL);
            cycle = 8;
            PC++;
            break;
        case 0x3B:
            SP[SP_indice]--;
            cycle = 8;
            PC++;
            break;
        case 0x3C:
            INC_reg_hi(AF);
            cycle = 4;
            PC++;
            break;
        case 0x3D:
            DEC_reg_hi(AF);
            cycle = 4;
            PC++;
            break;
        case 0x3E:
            LD_A(Mmu->read_ram(PC+1));
            PC += 2;
            cycle = 8;
            break;
        case 0x3F:
            CCF();
            cycle = 4;
            PC++;
            break;
        case 0x40:
            LD_reg_hi(BC, BC.hi());
            cycle = 4;
            PC++;
            break;
        case 0x41:
            LD_reg_hi(BC, BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0x42:
            LD_reg_hi(BC, DE.hi());
            cycle = 4;
            PC++;
            break;
        case 0x43:
            LD_reg_hi(BC, DE.lo());
            cycle = 4;
            PC++;
            break;
        case 0x44:
            LD_reg_hi(BC, HL.hi());
            cycle = 4;
            PC++;
            break;
        case 0x45:
            LD_reg_hi(BC, HL.lo());
            cycle = 4;
            PC++;
            break;
        case 0x46:
            LD_reg_hi(BC, Mmu->read_ram(HL.reg()));
            cycle = 8;
            PC++;
            break;
        case 0x47:
            LD_reg_hi(BC, AF.hi());
            cycle = 4;
            PC++;
            break;
        case 0x48:
            LD_reg_lo(BC, BC.hi());
            cycle = 4;
            PC++;
            break;
        case 0x49:
            LD_reg_lo(BC, BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0x4A:
            LD_reg_lo(BC, DE.hi());
            cycle = 4;
            PC++;
            break;
        case 0x4B:
            LD_reg_lo(BC, DE.lo());
            cycle = 4;
            PC++;
            break;
        case 0x4C:
            LD_reg_lo(BC, HL.hi());
            cycle = 4;
            PC++;
            break;
        case 0x4D:
            LD_reg_lo(BC, HL.lo());
            cycle = 4;
            PC++;
            break;
        case 0x4E:
            LD_reg_lo(BC, Mmu->read_ram(HL.reg()));
            cycle = 8;
            PC++;
            break;
        case 0x4F:
            BC.set_lo(AF.hi());
            PC++;
            cycle = 4;
            break;
        case 0x50:
            LD_reg_hi(DE, BC.hi());
            cycle = 4;
            PC++;
            break;
        case 0x51:
            LD_reg_hi(DE, BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0x52:
            LD_reg_hi(DE, DE.hi());
            cycle = 4;
            PC++;
            break;
        case 0x53:
            LD_reg_hi(DE, DE.lo());
            cycle = 4;
            PC++;
            break;
        case 0x54:
            LD_reg_hi(DE, HL.hi());
            cycle = 4;
            PC++;
            break;
        case 0x55:
            LD_reg_hi(DE, HL.lo());
            cycle = 4;
            PC++;
            break;
        case 0x56:
            LD_reg_hi(DE, Mmu->read_ram(HL.reg()));
            cycle = 8;
            PC++;
            break;
        case 0x57:
            DE.set_hi(AF.hi());
            PC++;
            cycle = 4;
            break;
        case 0x58:
            LD_reg_lo(DE, BC.hi());
            cycle = 4;
            PC++;
            break;
        case 0x59:
            LD_reg_lo(DE, BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0x5A:
            LD_reg_lo(DE, DE.hi());
            cycle = 4;
            PC++;
            break;
        case 0x5B:
            LD_reg_lo(DE, DE.lo());
            cycle = 4;
            PC++;
            break;
        case 0x5C:
            LD_reg_lo(DE, HL.hi());
            cycle = 4;
            PC++;
            break;
        case 0x5D:
            LD_reg_lo(DE, HL.lo());
            cycle = 4;
            PC++;
            break;
        case 0x5E:
            LD_reg_lo(DE, Mmu->read_ram(HL.reg()));
            cycle = 8;
            PC++;
            break;
        case 0x5F:
            LD_reg_lo(DE, AF.hi());
            cycle = 4;
            PC++;
            break;
        case 0x60:
            LD_reg_hi(HL, BC.hi());
            cycle = 4;
            PC++;
            break;
        case 0x61:
             LD_reg_hi(HL, BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0x62:
            LD_reg_hi(HL, DE.hi());
            cycle = 4;
            PC++;
            break;
        case 0x63:
            LD_reg_hi(HL, DE.lo());
            cycle = 4;
            PC++;
            break;
        case 0x64:
            LD_reg_hi(HL, HL.hi());
            cycle = 4;
            PC++;
            break;
        case 0x65:
            LD_reg_hi(HL, HL.lo());
            cycle = 4;
            PC++;
            break;
        case 0x66:
            LD_reg_hi(HL, Mmu->read_ram(HL.reg()));
            cycle = 8;
            PC++;
            break;
        case 0x67:
            HL.set_hi(AF.hi());
            cycle = 4;
            PC++;
            break;
        case 0x68:
            LD_reg_lo(HL, BC.hi());
            cycle = 4;
            PC++;
            break;
        case 0x69:
            LD_reg_lo(HL, BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0x6A:
            LD_reg_lo(HL, DE.hi());
            cycle = 4;
            PC++;
            break;
        case 0x6B:
            LD_reg_lo(HL, DE.lo());
            cycle = 4;
            PC++;
            break;
        case 0x6C:
            LD_reg_lo(HL, HL.hi());
            cycle = 4;
            PC++;
            break;
        case 0x6D:
            LD_reg_lo(HL, HL.lo());
            cycle = 4;
            PC++;
            break;
        case 0x6E:
            LD_reg_lo(HL, Mmu->read_ram(HL.reg()));
            cycle = 8;
            PC++;
            break;
        case 0x6F:
            LD_reg_lo(HL, AF.hi());
            cycle = 4;
            PC++;
            break;
        case 0x70:
            LD_Mem(HL.reg(), BC.hi());
            cycle = 4;
            PC++;
            break;
        case 0x71:
            LD_Mem(HL.reg(), BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0x72:
            LD_Mem(HL.reg(), DE.hi());
            cycle = 8;
            PC++;
            break;
        case 0x73:
            LD_Mem(HL.reg(), DE.lo());
            cycle = 8;
            PC++;
            break;
        case 0x74:
            LD_Mem(HL.reg(), HL.hi());
            cycle = 8;
            PC++;
            break;
        case 0x75:
            LD_Mem(HL.reg(), HL.lo());
            cycle = 8;
            PC++;
            break;
        case 0x76:
            {
            //TODO IMPLEMENT HALT
            PC++;
            cycle = 4;
            break;
            }
        case 0x77:
            LD_Mem(HL.reg(), AF.hi());
            cycle = 8;
            PC++;
            break;
        case 0x78:
            LD_A(BC.hi());
            cycle = 4;
            PC++;
            break;
        case 0x79:
            LD_A(BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0x7A:
            LD_A(DE.hi());
            cycle = 4;
            PC++;
            break;
        case 0x7B:
            LD_A(DE.lo());
            cycle = 4;
            PC++;
            break;
        case 0x7C:
            LD_A(HL.hi());
            cycle = 4;
            PC++;
            break;
        case 0x7D:
            LD_A(HL.lo());
            cycle  = 4;
            PC++;
            break;
        case 0x7E:
            LD_A(Mmu->read_ram(HL.reg()));
            cycle = 8;
            PC++;
            break;
        case 0x7F:
            LD_A(AF.hi());
            cycle = 4;
            PC++;
            break;
        case 0x80:
            ADD_A(BC.hi());
            cycle = 4;
            PC++;
            break;
        case 0x81:
            ADD_A(BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0x82:
            ADD_A(DE.hi());
            cycle = 4;
            PC++;
            break;
        case 0x83:
            ADD_A(DE.lo());
            cycle = 4;
            PC++;
            break;
        case 0x84:
            ADD_A(HL.hi());
            cycle = 4;
            PC++;
            break;
        case 0x85:
            ADD_A(HL.lo());
            cycle = 4;
            PC++;
            break;
        case 0x86:
            ADD_A(Mmu->read_ram(HL.reg()));
            cycle = 8;
            PC++;
            break;
        case 0x87:
            ADD_A(AF.hi());
            cycle = 4;
            PC++;
            break;
        case 0x88:
            ADC_A(BC.hi());
            cycle = 4;
            PC++;
            break;
        case 0x89:
            ADC_A(BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0x8A:
            ADC_A(DE.hi());
            cycle = 4;
            PC++;
            break;
        case 0x8B:
            ADC_A(DE.lo());
            cycle = 4;
            PC++;
            break;
        case 0x8C:
            ADC_A(HL.hi());
            cycle = 4;
            PC++;
            break;
        case 0x8D:
            ADC_A(HL.lo());
            cycle = 4;
            PC++;
            break;
        case 0x8E:
            ADC_A(Mmu->read_ram(HL.reg()));
            cycle = 8;
            PC++;
            break;
        case 0x8F:
            ADC_A(AF.hi());
            cycle = 4;
            PC++;
            break;
        case 0x90:
            SUB(BC.hi());
            cycle = 4;
            PC++;
            break;
        case 0x91:
            SUB(BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0x92:
            SUB(DE.hi());
            cycle = 4;
            PC++;
            break;
        case 0x93:
            SUB(DE.lo());
            cycle = 4;
            PC++;
            break;
        case 0x94:
            SUB(HL.hi());
            cycle = 4;
            PC++;
            break;
        case 0x95:
            SUB(HL.lo());
            cycle = 4;
            PC++;
            break;
        case 0x96:
            SUB(Mmu->read_ram(HL.reg()));
            cycle = 8;
            PC++;
            break;
        case 0x97:
            SUB(AF.hi());
            cycle = 4;
            PC++;
            break;
        case 0x98:
            SBC_A(BC.hi());
            cycle =4;
            PC++;
            break;
        case 0x99:
            SBC_A(BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0x9A:
            SBC_A(DE.hi());
            cycle =4;
            PC++;
            break;
        case 0x9B:
            SBC_A(DE.lo());
            cycle = 4;
            PC++;
            break;
        case 0x9C:
            SBC_A(HL.hi());
            cycle =4;
            PC++;
            break;
        case 0x9D:
            SBC_A(HL.lo());
            cycle = 4;
            PC++;
            break;
        case 0x9E:
            SBC_A(Mmu->read_ram(HL.reg()));
            cycle = 8;
            PC++;
            break;
        case 0x9F:
            SBC_A(AF.hi());
            cycle = 4;
            PC++;
            break;
        case 0xA0:
            AND_A(BC.hi());
            cycle = 4;
            PC++;
            break;
        case 0xA1:
            AND_A(BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0xA2:
            AND_A(DE.hi());
            cycle = 4;
            PC++;
            break;
        case 0xA3:
            AND_A(DE.lo());
            cycle = 4;
            PC++;
            break;
        case 0xA4:
            AND_A(HL.hi());
            cycle = 4;
            PC++;
            break;
        case 0xA5:
            AND_A(HL.lo());
            cycle = 4;
            PC++;
            break;
        case 0xA6:
            AND_A(Mmu->read_ram(HL.reg()));
            cycle = 8;
            PC++;
            break;
        case 0xA7:
            AND_A(AF.hi());
            cycle = 4;
            PC++;
            break;
        case 0xA8:
            XOR_A(BC.hi());
            cycle = 4;
            PC++;
            break;
        case 0xA9:
            XOR_A(BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0xAA:
            XOR_A(DE.hi());
            cycle = 4;
            PC++;
            break;
        case 0xAB:
            XOR_A(DE.lo());
            cycle = 4;
            PC++;
            break;
        case 0xAC:
            XOR_A(HL.hi());
            cycle = 4;
            PC++;
            break;
        case 0xAD:
            XOR_A(HL.lo());
            cycle = 4;
            PC++;
            break;
        case 0xAE:
            XOR_A(Mmu->read_ram(HL.reg()));
            cycle = 8;
            PC++;
            break;
        case 0xAF:
            XOR_A(AF.hi());
            cycle = 4;
            PC++;
            break;
        case 0xB0:
            OR_A(BC.hi());
            cycle = 4;
            PC++;
            break;
        case 0xB1:
            OR_A(BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0xB2:
            OR_A(DE.hi());
            cycle = 4;
            PC++;
            break;
        case 0xB3:
            OR_A(DE.lo());
            cycle = 4;
            PC++;
            break;
        case 0xB4:
            OR_A(HL.hi());
            cycle = 4;
            PC++;
            break;
        case 0xB5:
            OR_A(HL.lo());
            cycle = 4;
            PC++;
            break;
        case 0xB6:
            OR_A(Mmu->read_ram(HL.reg()));
            cycle = 8;
            PC++;
            break;
        case 0xB7:
            OR_A(AF.hi());
            cycle = 4;
            PC++;
            break;
        case 0xB8:
            CP(BC.hi());
            cycle = 4;
            PC++;
            break;
        case 0xB9:
            CP(BC.lo());
            cycle = 4;
            PC++;
            break;
        case 0xBA:
            CP(DE.hi());
            cycle = 4;
            PC++;
            break;
        case 0xBB:
            CP(DE.lo());
            cycle = 4;
            PC++;
            break;
        case 0xBC:
            CP(HL.hi());
            cycle = 4;
            PC++;
            break;
        case 0xBD:
            CP(HL.lo());
            cycle = 4;
            PC++;
            break;
        case 0xBE:
            CP(Mmu->read_ram(HL.reg()));
            cycle = 8;
            PC++;
            break;
        case 0xBF:
            CP(AF.hi());
            cycle = 4;
            PC++;
            break;
        case 0xC0:
            RET_NZ();
            cycle = 20;
            break;
        case 0xC1:
            POP_reg(BC);
            PC++;
            cycle = 12;
            break;
        case 0xC2:
            JP_NZ();
            cycle = 16;
            break;
        case 0xC3:
            JP(get_next_2byte());
            cycle = 16;
            break;
        case 0xC4:
            CALL_NZ();
            cycle = 24;
            break;
        case 0xC5:
            PUSH(BC);
            PC++;
            cycle = 16;
            break;
        case 0xC6:
            AND_A(Mmu->read_ram(PC+1));
            cycle = 8;
            PC+=2;
            break;
        case 0xC7:
            RST(0x00);
            cycle = 16;
            break;
        case 0xC8:
            RET_Z();
            cycle = 20;
            break;
        case 0xC9:
            RET();
            cycle = 16;
            break;
        case 0xCA:
            JP_Z();
            cycle = 16;
            break;
        //CB PREFIX FOR SPECIAL INSTRUCTION
        case 0xCC:
            CALL_Z();
            cycle = 24;
            break;
        case 0xCD:
            CALL(PC+3);
            cycle = 24;
            break;
        case 0xCE:
            ADC_A(Mmu->read_ram(PC+1));
            cycle = 8;
            PC++;
            break;
        case 0xCF:
            RST(0x08);
            cycle = 16;
            break;
        case 0xD0:
            RET_NC();
            cycle = 20;
            break;
        case 0xD1:
            POP_reg(DE);
            cycle = 12;
            PC++;
            break;
        case 0xD2:
            JP_NC();
            cycle = 16;
            break;
        case 0xD4:
            CALL_NC();
            cycle = 24;
            break;
        case 0xD5:
            PUSH(DE);
            cycle = 16;
            PC++;
            break;
        case 0xD6:
            SUB(Mmu->read_ram(PC+1));
            cycle = 8;
            PC+=2;
            break;
        case 0xD7:
            RST(0x10);
            cycle = 16;
            break;
        case 0xD8:
            RET_C();
            cycle = 20;
            break;
        case 0xD9:
            RETI();
            cycle = 16;
            break;
        case 0xDA:
            JP_C();
            cycle = 16;
            break;
        case 0xDC:
            CALL_C();
            cycle = 24;
            break;
        case 0xDE:
            SBC_A(Mmu->read_ram(PC+1));
            cycle = 8;
            PC+=2;
            break;
        case 0xDF:
            RST(0x18);
            cycle = 16;
            break;
        case 0xE0:
            Mmu->write_ram(Mmu->read_ram(PC+1) + 0xFF00, AF.hi());
            cycle = 12;
            PC+=2;
            break;
        case 0xE1:
            POP_reg(HL);
            cycle = 12;
            PC++;
            break;
        case 0xE2:
            Mmu->write_ram(0xFF00 + (uint16_t)BC.lo(), AF.hi());
            cycle = 8;
            PC++;
            break;
        case 0xE5:
            PUSH(HL);
            cycle = 16;
            PC++;
            break;
        case 0xE6:
            AND_A(Mmu->read_ram(PC+1));
            PC += 2;
            cycle = 8;
            break;
        case 0xE7:
            RST(0x20);
            cycle = 16;
            break;
        case 0xE8:
            ADD_SP(Mmu->read_ram(PC+1));
            PC+=2;
            cycle = 16;
            break;
        case 0xE9:
            JP(HL.reg());
            cycle = 4;
            break;
        case 0xEA:
            LD_Mem(get_next_2byte(), AF.hi());
            cycle = 16;
            PC += 3;
            break;
        case 0xEE:
            XOR_A(Mmu->read_ram(PC+1));
            PC+=2;
            cycle = 8;
            break;
        case 0xEF:
            RST(0x28);
            cycle = 16;
            break;
        case 0xF0:
            LDH_A(PC+1);
            cycle = 12;
            PC+=2;
            break;
        case 0xF1:
            POP_reg(AF);
            cycle = 12;
            PC++;
            break;
        case 0xF2:
            LD_A(Mmu->read_ram((uint16_t)BC.lo()));
            cycle = 8;
            PC++;
            break;
        case 0xF3:
            //DISABLE INTERRUPT
            enable_interrupt = false;
            cycle = 4;
            PC++;
            break;
        case 0xF5:
            PUSH(AF);
            cycle = 16;
            PC++;
            break;
        case 0xF6:
            OR_A(Mmu->read_ram(PC+1));
            PC+=2;
            cycle = 8;
            break;
        case 0xF7:
            RST(0x30);
            cycle =16;
            break;
        case 0xF8:
            LD_reg(HL, SP[SP_indice] + Mmu->read_ram(PC+1));
            cycle = 12;
            PC+=2;
            break;
        case 0xF9:
            push_SP(HL.reg());
            cycle = 8;
            PC++;
            break;
        case 0xFA:
            LD_A(Mmu->read_ram(get_next_2byte()));
            cycle = 16;
            PC+=3;
            break;
        case 0xFB:
            //ENABLE INTERRUPT
            enable_interrupt = true;
            cycle = 4;
            PC++;
            break;
        case 0xFE:
            CP(Mmu->read_ram(PC+1));
            cycle = 8;
            PC += 2;
            break;
        case 0xFF:
            RST(0x38);
            cycle = 16;
            break;

        /*-----SPECIAL OPCODES-----*/

        case 0xCB:
        {
            PC++;
            switch (Mmu->read_ram(PC))
            {
                case 0x00:
                    RLC_hi(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x01:
                    RLC_lo(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x02:
                    RLC_hi(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x03:
                    RLC_lo(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x04:
                    RLC_hi(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x05:
                    RLC_lo(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x06:
                    RLC_HL();
                    cycle = 16;
                    PC++;
                    break;
                case 0x07:
                    RLC_hi(AF);
                    cycle = 8;
                    PC++;
                    break;
                case 0x08:
                    RRC_hi(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x09:
                    RRC_lo(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x0A:
                    RRC_hi(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x0B:
                    RRC_lo(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x0C:
                    RRC_hi(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x0D:
                    RRC_lo(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x0E:
                    RRC_HL();
                    cycle = 16;
                    PC++;
                    break;
                case 0x0F:
                    RRC_hi(AF);
                    cycle = 8;
                    PC++;
                    break;
                case 0x10:
                    RL_reg_hi(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x11:
                    RL_reg_lo(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x12:
                    RL_reg_hi(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x13:
                    RL_reg_lo(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x14:
                    RL_reg_hi(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x15:
                    RL_reg_lo(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x16:
                    RL_n(HL.reg());
                    cycle = 16;
                    PC++;
                    break;
                case 0x17:
                    RL_A();
                    cycle = 8;
                    PC++;
                    break;
                case 0x18:
                    RR_reg_hi(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x19:
                    RR_reg_lo(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x1A:
                    RR_reg_hi(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x1B:
                    RR_reg_lo(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x1C:
                    RR_reg_hi(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x1D:
                    RR_reg_lo(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x1E:
                    RR_n(HL.reg());
                    cycle = 16;
                    PC++;
                    break;
                case 0x1F:
                    RR_reg_hi(AF);
                    cycle = 8;
                    PC++;
                    break;
                case 0x20:
                    SLA_hi(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x21:
                    SLA_lo(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x22:
                    SLA_hi(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x23:
                    SLA_lo(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x24:
                    SLA_hi(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x25:
                    SLA_lo(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x26:
                    SLA_HL();
                    cycle = 16;
                    PC++;
                    break;
                case 0x27:
                    SLA_hi(AF);
                    cycle = 8;
                    PC++;
                    break;
                case 0x28:
                    SRA_hi(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x29:
                    SRA_lo(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x2A:
                    SRA_hi(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x2B:
                    SRA_lo(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x2C:
                    SRA_hi(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x2D:
                    SRA_lo(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x2E:
                    SRA_HL();
                    cycle = 16;
                    PC++;
                    break;
                case 0x2F:
                    SRA_hi(AF);
                    cycle = 8;
                    PC++;
                    break;
                case 0x30:
                    SWAP_reg_hi(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x31:
                    SWAP_reg_lo(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x32:
                    SWAP_reg_hi(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x33:
                    SWAP_reg_lo(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x34:
                    SWAP_reg_hi(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x35:
                    SWAP_reg_lo(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x36:
                    SWAP_HL();
                    cycle = 16;
                    PC++;
                    break;
                case 0x37:
                    SWAP_reg_hi(AF);
                    cycle = 8;
                    PC++;
                    break;
                case 0x38:
                    SRL_reg_hi(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x39:
                    SRL_reg_lo(BC);
                    cycle = 8;
                    PC++;
                    break;
                case 0x3A:
                    SRL_reg_hi(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x3B:
                    SRL_reg_lo(DE);
                    cycle = 8;
                    PC++;
                    break;
                case 0x3C:
                    SRL_reg_hi(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x3D:
                    SRL_reg_lo(HL);
                    cycle = 8;
                    PC++;
                    break;
                case 0x3E:
                    SRL_HL();
                    cycle = 16;
                    PC++;
                    break;
                case 0x3F:
                    SRL_reg_hi(AF);
                    cycle = 8;
                    PC++;
                    break;
                case 0x40:
                    BIT(BC.hi(), 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0x41:
                    BIT(BC.lo(), 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0x42:
                    BIT(DE.hi(), 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0x43:
                    BIT(DE.lo(), 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0x44:
                    BIT(HL.hi(), 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0x45:
                    BIT(HL.lo(), 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0x46:
                    BIT(Mmu->read_ram(HL.reg()), 0);
                    cycle = 16;
                    PC++;
                    break;
                case 0x47:
                    BIT(AF.hi(), 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0x48:
                    BIT(BC.hi(), 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0x49:
                    BIT(BC.lo(), 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0x4A:
                    BIT(DE.hi(), 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0x4B:
                    BIT(DE.lo(), 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0x4C:
                    BIT(HL.hi(), 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0x4D:
                    BIT(HL.lo(), 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0x4E:
                    BIT(Mmu->read_ram(HL.reg()), 1);
                    cycle = 16;
                    PC++;
                    break;
                case 0x4F:
                    BIT(AF.hi(), 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0x50:
                    BIT(BC.hi(), 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0x51:
                    BIT(BC.lo(), 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0x52:
                    BIT(DE.hi(), 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0x53:
                    BIT(DE.lo(), 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0x54:
                    BIT(HL.hi(), 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0x55:
                    BIT(HL.lo(), 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0x56:
                    BIT(Mmu->read_ram(HL.reg()), 2);
                    cycle = 16;
                    PC++;
                    break;
                case 0x57:
                    BIT(AF.hi(), 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0x58:
                    BIT(BC.hi(), 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0x59:
                    BIT(BC.lo(), 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0x5A:
                    BIT(DE.hi(), 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0x5B:
                    BIT(DE.lo(), 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0x5C:
                    BIT(HL.hi(), 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0x5D:
                    BIT(HL.lo(), 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0x5E:
                    BIT(Mmu->read_ram(HL.reg()), 3);
                    cycle = 16;
                    PC++;
                    break;
                case 0x5F:
                    BIT(AF.hi(), 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0x60:
                    BIT(BC.hi(), 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0x61:
                    BIT(BC.lo(), 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0x62:
                    BIT(DE.hi(), 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0x63:
                    BIT(DE.lo(), 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0x64:
                    BIT(HL.hi(), 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0x65:
                    BIT(HL.lo(), 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0x66:
                    BIT(Mmu->read_ram(HL.reg()), 4);
                    cycle = 16;
                    PC++;
                    break;
                case 0x67:
                    BIT(AF.hi(), 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0x68:
                    BIT(BC.hi(), 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0x69:
                    BIT(BC.lo(), 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0x6A:
                    BIT(DE.hi(), 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0x6B:
                    BIT(DE.lo(), 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0x6C:
                    BIT(HL.hi(), 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0x6D:
                    BIT(HL.lo(), 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0x6E:
                    BIT(Mmu->read_ram(HL.reg()), 5);
                    cycle = 16;
                    PC++;
                    break;
                case 0x6F:
                    BIT(AF.hi(), 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0x70:
                    BIT(BC.hi(), 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0x71:
                    BIT(BC.lo(), 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0x72:
                    BIT(DE.hi(), 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0x73:
                    BIT(DE.lo(), 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0x74:
                    BIT(HL.hi(), 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0x75:
                    BIT(HL.lo(), 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0x76:
                    BIT(Mmu->read_ram(HL.reg()), 6);
                    cycle = 16;
                    PC++;
                    break;
                case 0x77:
                    BIT(AF.hi(), 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0x78:
                    BIT(BC.hi(), 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0x79:
                    BIT(BC.lo(), 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0x7A:
                    BIT(DE.hi(), 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0x7B:
                    BIT(DE.lo(), 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0x7C:
                    BIT(HL.hi(), 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0x7D:
                    BIT(HL.lo(), 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0x7E:
                    BIT(Mmu->read_ram(HL.reg()), 7);
                    cycle = 16;
                    PC++;
                    break;
                case 0x7F:
                    BIT(AF.hi(), 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0x80:
                    RES_hi(BC, 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0x81:
                    RES_lo(BC, 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0x82:
                    RES_hi(DE, 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0x83:
                    RES_lo(DE, 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0x84:
                    RES_hi(HL, 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0x85:
                    RES_lo(HL, 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0x86:
                    RES_HL(0);
                    cycle = 16;
                    PC++;
                    break;
                case 0x87:
                    RES_hi(AF, 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0x88:
                    RES_hi(BC, 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0x89:
                    RES_lo(BC, 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0x8A:
                    RES_hi(DE, 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0x8B:
                    RES_lo(DE, 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0x8C:
                    RES_hi(HL, 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0x8D:
                    RES_lo(HL, 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0x8E:
                    RES_HL(1);
                    cycle = 16;
                    PC++;
                    break;
                case 0x8F:
                    RES_hi(AF, 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0x90:
                    RES_hi(BC, 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0x91:
                    RES_lo(BC, 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0x92:
                    RES_hi(DE, 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0x93:
                    RES_lo(DE, 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0x94:
                    RES_hi(HL, 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0x95:
                    RES_lo(HL, 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0x96:
                    RES_HL(2);
                    cycle = 16;
                    PC++;
                    break;
                case 0x97:
                    RES_hi(AF, 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0x98:
                    RES_hi(BC, 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0x99:
                    RES_lo(BC, 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0x9A:
                    RES_hi(DE, 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0x9B:
                    RES_lo(DE, 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0x9C:
                    RES_hi(HL, 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0x9D:
                    RES_lo(HL, 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0x9E:
                    RES_HL(3);
                    cycle = 16;
                    PC++;
                    break;
                case 0x9F:
                    RES_hi(AF, 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0xA0:
                    RES_hi(BC, 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0xA1:
                    RES_lo(BC, 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0xA2:
                    RES_hi(DE, 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0xA3:
                    RES_lo(DE, 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0xA4:
                    RES_hi(HL, 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0xA5:
                    RES_lo(HL, 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0xA6:
                    RES_HL(4);
                    cycle = 16;
                    PC++;
                    break;
                case 0xA7:
                    RES_hi(AF, 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0xA8:
                    RES_hi(BC, 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0xA9:
                    RES_lo(BC, 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0xAA:
                    RES_hi(DE, 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0xAB:
                    RES_lo(DE, 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0xAC:
                    RES_hi(HL, 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0xAD:
                    RES_lo(HL, 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0xAE:
                    RES_HL(5);
                    cycle = 16;
                    PC++;
                    break;
                case 0xAF:
                    RES_hi(AF, 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0xB0:
                    RES_hi(BC, 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0xB1:
                    RES_lo(BC, 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0xB2:
                    RES_hi(DE, 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0xB3:
                    RES_lo(DE, 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0xB4:
                    RES_hi(HL, 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0xB5:
                    RES_lo(HL, 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0xB6:
                    RES_HL(6);
                    cycle = 16;
                    PC++;
                    break;
                case 0xB7:
                    RES_hi(AF, 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0xB8:
                    RES_hi(BC, 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0xB9:
                    RES_lo(BC, 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0xBA:
                    RES_hi(DE, 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0xBB:
                    RES_lo(DE, 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0xBC:
                    RES_hi(HL, 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0xBD:
                    RES_lo(HL, 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0xBE:
                    RES_HL(7);
                    cycle = 16;
                    PC++;
                    break;
                case 0xBF:
                    RES_hi(AF, 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0xC0:
                    SET_hi(BC, 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0xC1:
                    SET_lo(BC, 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0xC2:
                    SET_hi(DE, 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0xC3:
                    SET_lo(DE, 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0xC4:
                    SET_hi(HL, 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0xC5:
                    SET_lo(HL, 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0xC6:
                    SET_HL(0);
                    cycle = 16;
                    PC++;
                    break;
                case 0xC7:
                    SET_hi(AF, 0);
                    cycle = 8;
                    PC++;
                    break;
                case 0xC8:
                    SET_hi(BC, 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0xC9:
                    SET_lo(BC, 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0xCA:
                    SET_hi(DE, 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0xCB:
                    SET_lo(DE, 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0xCC:
                    SET_hi(HL, 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0xCD:
                    SET_lo(HL, 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0xCE:
                    SET_HL(1);
                    cycle = 16;
                    PC++;
                    break;
                case 0xCF:
                    SET_hi(AF, 1);
                    cycle = 8;
                    PC++;
                    break;
                case 0xD0:
                    SET_hi(BC, 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0xD1:
                    SET_lo(BC, 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0xD2:
                    SET_hi(DE, 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0xD3:
                    SET_lo(DE, 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0xD4:
                    SET_hi(HL, 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0xD5:
                    SET_lo(HL, 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0xD6:
                    SET_HL(2);
                    cycle = 16;
                    PC++;
                    break;
                case 0xD7:
                    SET_hi(AF, 2);
                    cycle = 8;
                    PC++;
                    break;
                case 0xD8:
                    SET_hi(BC, 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0xD9:
                    SET_lo(BC, 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0xDA:
                    SET_hi(DE, 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0xDB:
                    SET_lo(DE, 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0xDC:
                    SET_hi(HL, 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0xDD:
                    SET_lo(HL, 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0xDE:
                    SET_HL(3);
                    cycle = 16;
                    PC++;
                    break;
                case 0xDF:
                    SET_hi(AF, 3);
                    cycle = 8;
                    PC++;
                    break;
                case 0xE0:
                    SET_hi(BC, 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0xE1:
                    SET_lo(BC, 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0xE2:
                    SET_hi(DE, 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0xE3:
                    SET_lo(DE, 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0xE4:
                    SET_hi(HL, 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0xE5:
                    SET_lo(HL, 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0xE6:
                    SET_HL(4);
                    cycle = 16;
                    PC++;
                    break;
                case 0xE7:
                    SET_hi(AF, 4);
                    cycle = 8;
                    PC++;
                    break;
                case 0xE8:
                    SET_hi(BC, 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0xE9:
                    SET_lo(BC, 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0xEA:
                    SET_hi(DE, 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0xEB:
                    SET_lo(DE, 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0xEC:
                    SET_hi(HL, 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0xED:
                    SET_lo(HL, 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0xEE:
                    SET_HL(5);
                    cycle = 16;
                    PC++;
                    break;
                case 0xEF:
                    SET_hi(AF, 5);
                    cycle = 8;
                    PC++;
                    break;
                case 0xF0:
                    SET_hi(BC, 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0xF1:
                    SET_lo(BC, 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0xF2:
                    SET_hi(DE, 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0xF3:
                    SET_lo(DE, 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0xF4:
                    SET_hi(HL, 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0xF5:
                    SET_lo(HL, 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0xF6:
                    SET_HL(6);
                    cycle = 16;
                    PC++;
                    break;
                case 0xF7:
                    SET_hi(AF, 6);
                    cycle = 8;
                    PC++;
                    break;
                case 0xF8:
                    SET_hi(BC, 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0xF9:
                    SET_lo(BC, 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0xFA:
                    SET_hi(DE, 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0xFB:
                    SET_lo(DE, 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0xFC:
                    SET_hi(HL, 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0xFD:
                    SET_lo(HL, 7);
                    cycle = 8;
                    PC++;
                    break;
                case 0xFE:
                    SET_HL(7);
                    cycle = 16;
                    PC++;
                    break;
                case 0xFF:
                    SET_hi(AF, 7);
                    cycle = 8;
                    PC++;
                    break;

                default:
                {
                    ofstream file("./debug.txt");
                    file << "Unsupported 0xCB Instruction : " << hex << (int)Mmu->read_ram(PC);
                    exit(0xCB);
                    break;
                }
            }
            break;
        }
        default:
            ofstream file("./debug.txt");
            file << "Unsupported Instruction : " << hex << (int)Mmu->read_ram(PC);
            break;
            exit(0);
            break;
    }
}
void Cpu::pop_SP()
{
    SP[SP_indice] = 0x0;
    if (SP_indice > 0)
        SP_indice--;
}

void Cpu::push_SP(uint16_t value)
{
    if (SP_indice == 0 && SP[SP_indice] == 0x0)
        SP[SP_indice] = value;
    else
    {
        SP_indice++;
        SP[SP_indice] = value;
    }
}

bool Cpu::get_gpu_debug()
{
    return gpu_debug;
}

void Cpu::SUB(uint8_t value)//CHANGED
{
    if (AF.hi() - value < 0x00)
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    if ((AF.hi() & 0x0F) - (value & 0x0F) < 0x00)
        AF.set_halfcarryflag();
    else
        AF.clear_halfcarryflag();
    AF.set_hi(AF.hi() - value);
    if (AF.hi() == 0x0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.set_subflag();
}

void Cpu::CP(uint8_t value)
{
    if ((uint16_t)AF.hi() - (uint16_t)value < 0x0)
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    if ((AF.hi() & 0x0F) - (value & 0x0F) < 0x0)
        AF.set_halfcarryflag();
    else
        AF.clear_halfcarryflag();
    if (AF.hi() - value == 0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.set_subflag();
}

void Cpu::LD_A(uint8_t value)
{
    AF.set_hi(value);
}

void Cpu::ADD_A(uint8_t value)
{
    if (AF.hi() + value >= 0xFF)
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    if (((AF.hi() & 0x0F) + (value & 0x0F)) >= 0x0F)
        AF.set_halfcarryflag();
    else
        AF.clear_halfcarryflag();
    uint8_t tmp = (AF.hi() + value);
    AF.set_hi(tmp);
    if ((int)AF.hi() == 0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.clear_subflag();
}

void Cpu::JP(uint16_t value)
{
    PC = value;
}

void Cpu::JP_NZ()
{
    if (!AF.get_zflag())
        PC = get_next_2byte();
    else PC += 3;
}

void Cpu::JP_Z()
{
    if (AF.get_zflag())
        PC = get_next_2byte();
    else PC += 3;
}

void Cpu::JP_NC()
{
    if (!AF.get_carryflag())
        PC = get_next_2byte();
    else PC += 3;
}

void Cpu::JP_C()
{
    if (AF.get_carryflag())
        PC = get_next_2byte();
    else PC += 3;
}


void Cpu::LD_reg(Register &Reg,uint16_t value)
{
    Reg.set(value);
}

void Cpu::LD_reg_hi(Register &Reg,uint8_t value)
{
    Reg.set_hi(value);
}

void Cpu::LD_reg_lo(Register &Reg,uint8_t value)
{
    Reg.set_lo(value);
}

void Cpu::RL_reg_hi(Register &Reg)
{
    uint8_t C = Reg.hi();
    int old_carry;
    if (AF.get_carryflag())
        old_carry = 1;
    else
        old_carry = 0;
    if (C & (1 << 7))
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    C = C << 1;
    if (old_carry)
        C |= 0x1;
    Reg.set_hi(C);
    if (Reg.hi() == 0x0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::RL_reg_lo(Register &Reg)
{
    uint8_t C = Reg.lo();
    int old_carry;
    if (AF.get_carryflag())
        old_carry = 1;
    else
        old_carry = 0;
    if (C & (1 << 7))
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    C = C << 1;
    if (old_carry)
        C |= 0x1;
    Reg.set_lo(C);
    if (Reg.lo() == 0x0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::RL_n(uint16_t adrr)
{
    uint8_t C = Mmu->read_ram(adrr);
    int old_carry;
    if (AF.get_carryflag())
        old_carry = 1;
    else
        old_carry = 0;
    if (C & (1 << 7))
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    C = C << 1;
    if (old_carry)
        C |= 0x1;
    Mmu->write_ram(adrr, C);
    if (Mmu->read_ram(adrr) == 0x0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::RR_reg_hi(Register &Reg)
{
    uint8_t C = Reg.hi();
    Reg.set_hi(Reg.hi() >> 1);
    uint8_t tmp;
    if (AF.get_carryflag())
        tmp = 1;
    else tmp = 0;
    Reg.set_hi(modify_bit(Reg.hi(), 7, tmp));
    if (C & 0b00000001)
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    if (Reg.hi() == 0x0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::RR_reg_lo(Register &Reg)
{
    uint8_t C = Reg.lo();
    Reg.set_lo(Reg.lo() >> 1);
    uint8_t tmp;
    if (AF.get_carryflag())
        tmp = 1;
    else tmp = 0;
    Reg.set_lo(modify_bit(Reg.lo(), 7, tmp));
    if (C & 0b00000001)
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    if (Reg.lo() == 0x0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::RR_n(uint16_t adrr)
{
    uint8_t C = Mmu->read_ram(adrr);
    uint8_t V = Mmu->read_ram(adrr);
    V = V >> 1;
    uint8_t tmp;
    if (AF.get_carryflag())
        tmp = 1;
    else tmp = 0;
    V = modify_bit(V, 7, tmp);
    Mmu->write_ram(adrr, V);
    if (C & 0b00000001)
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    if (Mmu->read_ram(adrr) == 0x0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}



void Cpu::ADD_HL(uint16_t value) //NOT_SURE
{
    HL.set(HL.reg() + value);
    AF.clear_subflag();
    if ((uint32_t)HL.reg() + (uint32_t)value > 0xFFFF)
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    if ((HL.lo() & 0x0F) + (HL.hi() & 0x0F) > 0xFF)
        AF.set_halfcarryflag();
    else
        AF.clear_halfcarryflag();
}

void Cpu::SBC_A(uint8_t value)
{
    int carry;
    if (AF.get_carryflag())
        carry = 1;
    else
        carry = 0;
    if((int16_t)AF.hi() - (int16_t)value - carry < 0x00)
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    if ((AF.hi() & 0x0F) - (value & 0x0F) - carry < 0x00)
        AF.set_halfcarryflag();
    else
        AF.clear_halfcarryflag();
    AF.set_hi(AF.hi() - (value + carry));
    if (AF.hi() == 0x0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.set_subflag();
}

void Cpu::SRL_HL()
{
    uint8_t tmp = Mmu->read_ram(HL.reg());
    Mmu->write_ram(HL.reg(), Mmu->read_ram(HL.reg()) >> 1);
    if (tmp & 0x01)
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    if (Mmu->read_ram(HL.reg()) == 0x0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::SRL_reg_hi(Register &Reg)
{
    uint8_t tmp = Reg.hi();
    Reg.set_hi(Reg.lo() >> 1);
    if (tmp & 0x01)
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    if (Reg.hi() == 0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::SRL_reg_lo(Register &Reg)
{
    uint8_t tmp = Reg.lo();
    Reg.set_lo(Reg.lo() >> 1);
    if (tmp & 0x01)
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    if (Reg.lo() == 0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::RL_A()
{
    uint8_t A = AF.hi();
    int old_carry;
    if (AF.get_carryflag())
        old_carry = 1;
    else
        old_carry = 0;
    if (A & (1 << 7))
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    A = A << 1;
    if (old_carry)
        A |= 0x1;
    AF.set_hi(A);
    if (AF.hi() == 0x0)
        AF.set_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::AND_A(uint8_t value)
{
    AF.set_hi(AF.hi() & value );
    if (AF.hi() == 0x0)
        AF.set_zflag();
    else
        AF.clear_zflag();
    AF.clear_subflag();
    AF.set_halfcarryflag();
    AF.clear_carryflag();
}

void Cpu::RST(uint8_t value)
{
    SP_indice++;
    SP[SP_indice] = PC + 1;
    PC = 0x0000 + (uint16_t)value;
}

void Cpu::POP_reg(Register &Reg)
{
    Reg.set(SP[SP_indice]);
    pop_SP();
}

void Cpu::POP_reg(uint16_t &value)
{
    value = SP[SP_indice];
    pop_SP();
}


void Cpu::RET_Z()
{
    if (AF.get_zflag())
    {
        PC = SP[SP_indice];
        pop_SP();
    }
    else
        PC++;
}

void Cpu::RET_NZ()
{
    if (!AF.get_zflag())
    {
        PC = SP[SP_indice];
        pop_SP();
    }
    else
        PC++;
}

void Cpu::RET_C()
{
    if (AF.get_carryflag())
    {
        PC = SP[SP_indice];
        pop_SP();
    }
    else
        PC++;
}

void Cpu::RET_NC()
{
    if (!AF.get_carryflag())
    {
        PC = SP[SP_indice];
        pop_SP();
    }
    else
        PC++;
}

void Cpu::LD_Mem(uint16_t adrr, uint8_t value)
{
    Mmu->write_ram(adrr, value);
}

void Cpu::debug()
{
    cout << "---------------------------------" << endl;
    cout << "AF = ";
    AF.show();
    if (AF.get_zflag())
        cout << ",    Z = 1" << endl;
    else
        cout << ",    Z = 0" << endl;
    cout << "BC = ";
    BC.show();
    if (AF.get_subflag())
        cout << ",    N = 1" << endl;
    else
        cout << ",    N = 0" << endl;
    cout << "DE = ";
    DE.show();
    if (AF.get_halfcarryflag())
        cout << ",    H = 1" << endl;
    else
        cout << ",    H = 0" << endl;
    cout << "HL = ";
    HL.show();
    if (AF.get_carryflag())
        cout << ",    C = 1" << endl;
    else
        cout << ",    C = 0" << endl;
    cout << "PC executed = " << hex << (int)Mmu->read_ram(prev_CP) << " at " << hex << (int)prev_CP << endl;
    cout << "Current PC = " << hex << (int)PC << ", Memory[PC] = " << (int)Mmu->read_ram(PC) << endl;
    cout << "Currnt SP = " << hex << (int)SP[SP_indice] << endl ;
    cout << "RAM[FF44] = " << hex << (int)Mmu->read_ram(0xFF43) << hex << (int)Mmu->read_ram(0xFF44) << endl;
    cout << "---------------------------------" << endl;
}

void Cpu::JR_C(uint8_t value)
{
    if (AF.get_carryflag())
        PC += (int8_t)value;
    PC+=2;
}

void Cpu::JR_NC(uint8_t value)
{
    if (!AF.get_carryflag())
        PC += (int8_t)value;
    PC+=2;
}

void Cpu::JR_Z(uint8_t value)
{
    if (AF.get_carryflag())
        PC += (int8_t)value;
    PC+=2;
}

void Cpu::JR_NZ(uint8_t value)
{
    if (!AF.get_carryflag())
        PC += (int8_t)value;
    PC+=2;
}

void Cpu::SWAP_reg_hi(Register &Reg)
{
    uint8_t tmp = Reg.hi();
    Reg.set_hi(Reg.hi() << 4);
    Reg.set_hi(Reg.hi() | (tmp  >> 4));
    if (Reg.hi() == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_subflag();
    AF.clear_halfcarryflag();
    AF.clear_carryflag();
}

void Cpu::SWAP_reg_lo(Register &Reg)
{
    uint8_t tmp = Reg.lo();
    Reg.set_lo(Reg.lo() << 4);
    Reg.set_lo(Reg.lo() | (tmp  >> 4));
    if (Reg.lo() == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_subflag();
    AF.clear_halfcarryflag();
    AF.clear_carryflag();
}

void Cpu::SWAP_HL()
{
    uint8_t tmp = Mmu->read_ram(HL.reg());
    uint8_t tmp2 = Mmu->read_ram(HL.reg());
    tmp2 <<= 4;
    tmp2 = tmp2 | (tmp  >> 4);
    Mmu->write_ram(HL.reg(), tmp2);
    if (tmp2 == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_subflag();
    AF.clear_halfcarryflag();
    AF.clear_carryflag();
}

void Cpu::PUSH(Register &Reg)
{
    SP_indice ++;
    SP[SP_indice] = Reg.reg();
}

void Cpu::CALL_NZ()
{
    if (!AF.get_zflag())
    {
        push_SP(PC+3);
        PC = get_next_2byte();
    }
    else PC++;
}

void Cpu::CALL_Z()
{
    if (AF.get_zflag())
    {
        push_SP(PC+3);
        PC = get_next_2byte();
    }
    else PC++;
}

void Cpu::CALL_NC()
{
    if (!AF.get_carryflag())
    {
        push_SP(PC+3);
        PC = get_next_2byte();
    }
    else PC++;
}

void Cpu::CALL_C()
{
    if (AF.get_carryflag())
    {
        push_SP(PC+3);
        PC = get_next_2byte();
    }
    else PC++;
}

void Cpu::CALL(uint16_t value)
{
    push_SP(value);
    PC = get_next_2byte();
}

uint8_t Cpu::modify_bit(uint8_t n, uint8_t pos, uint8_t value)
{
    int mask = 1 << pos;
    return (n & ~mask) | ((value << pos) & mask);
}

void Cpu::CPL()
{
    uint8_t tmp = AF.hi();
    tmp = ~tmp;
    AF.set_hi(tmp);
    AF.set_subflag();
    AF.set_halfcarryflag();
}

void Cpu::RLCA()
{
    uint8_t old_A = AF.hi();
    AF.set_hi(AF.hi() << 1);
    AF.set_hi(AF.hi() | (old_A >> 7));
    if (old_A >> 7)
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
    AF.clear_zflag();
}

void Cpu::RRCA()
{
    uint8_t bit0 = AF.hi() & 0b00000001;
    AF.set_hi(AF.hi() >> 1);
    AF.set_hi(modify_bit(AF.hi(), 7, bit0));
    if (bit0)
        AF.set_carryflag();
    else
        AF.clear_carryflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
    AF.clear_zflag();
}

void Cpu::SCF()
{
    AF.set_carryflag();
    AF.clear_subflag();
    AF.clear_halfcarryflag();
}

void Cpu::CCF()
{
    AF.clear_subflag();
    AF.clear_halfcarryflag();
    if (BC.lo() != 0)
        AF.set_carryflag();
    else AF.clear_carryflag();
}

void Cpu::ADC_A(uint8_t value)
{
    uint8_t tmp;
    if (AF.get_carryflag())
        tmp = 0x1;
    else tmp = 0x0;
    if ((uint16_t)AF.hi() + (uint16_t)value + tmp > 0xFF)
        AF.set_carryflag();
    else AF.clear_carryflag();
    if (((uint8_t)AF.hi() << 4) + ((uint8_t)value << 4) + tmp > 0xFF)
        AF.set_halfcarryflag();
    else AF.clear_halfcarryflag();
    AF.set_hi(AF.hi() + (value + tmp));
    if (AF.hi() == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_subflag();
}

void Cpu::RET()
{
    PC = SP[SP_indice];
    pop_SP();
}

void Cpu::RETI()
{
    PC = SP[SP_indice];
    pop_SP();
    enable_interrupt = true;
}

void Cpu::LDH_A(uint16_t value)
{
    AF.set_hi(Mmu->read_ram(0xFF00 + Mmu->read_ram(value)));
}

void Cpu::LDH_Mem(uint8_t value)
{
    Mmu->write_ram(0xFF00 + (uint16_t)value, AF.hi());
}

void Cpu::ADD_SP(uint8_t value)
{
    SP[SP_indice] += (int8_t)value;
    AF.clear_zflag();
    AF.clear_subflag();
    if ((uint32_t)SP[SP_indice] + (uint32_t)value > 0xFFFF)
        AF.set_carryflag();
    else AF.clear_carryflag();
    if (SP[SP_indice] > 0x0F)
    AF.set_halfcarryflag();
    else AF.clear_halfcarryflag();
}

void Cpu::SET_HL(int index)
{
    Mmu->write_ram(HL.reg(), (Mmu->read_ram(HL.reg()) | (1 << index)));
}

void Cpu::SET_hi(Register &Reg, int index)
{
    Reg.set_hi(Reg.hi() | (1 << index));
}

void Cpu::SET_lo(Register &Reg, int index)
{
    Reg.set_lo(Reg.lo() | (1 << index));
}

void Cpu::RLC_hi(Register &Reg)
{
    if (Reg.hi()  & (1 << 7))
        AF.set_carryflag();
    else AF.clear_carryflag();
    Reg.set_hi(Reg.hi() << 1);
    if (AF.get_carryflag() == 1)
        Reg.set_hi(Reg.hi() | (1<<0));
    if (Reg.hi() == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::RLC_lo(Register &Reg)
{
    if (Reg.lo()  & (1 << 7))
        AF.set_carryflag();
    else AF.clear_carryflag();
    Reg.set_lo(Reg.lo() << 1);
    if (AF.get_carryflag() == 1)
        Reg.set_lo(Reg.lo() | (1<<0));
    if (Reg.lo() == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::RLC_HL()
{
    if (Mmu->read_ram(HL.reg())  & (1 << 7))
        AF.set_carryflag();
    else AF.clear_carryflag();
    Mmu->write_ram(HL.reg(), Mmu->read_ram(HL.reg()) << 1);
    if (AF.get_carryflag() == 1)
       Mmu->write_ram(HL.reg(), Mmu->read_ram(HL.reg()) | (1 << 0));
    if (Mmu->read_ram(HL.reg()) == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::RRC_hi(Register &Reg)
{
    if (Reg.hi()  & (1 << 0))
        AF.set_carryflag();
    else AF.clear_carryflag();
    Reg.set_hi(Reg.hi() >> 1);
    if (AF.get_carryflag() == 1)
        Reg.set_hi(Reg.hi() | (1<<7));
    if (Reg.hi() == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::RRC_lo(Register &Reg)
{
    if (Reg.lo()  & (1 << 0))
        AF.set_carryflag();
    else AF.clear_carryflag();
    Reg.set_lo(Reg.lo() >> 1);
    if (AF.get_carryflag() == 1)
        Reg.set_lo(Reg.lo() | (1<<7));
    if (Reg.lo() == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::RRC_HL()
{
    if (Mmu->read_ram(HL.reg())  & (1 << 0))
        AF.set_carryflag();
    else AF.clear_carryflag();
    Mmu->write_ram(HL.reg(), Mmu->read_ram(HL.reg()) >> 1);
    if (AF.get_carryflag() == 1)
       Mmu->write_ram(HL.reg(), Mmu->read_ram(HL.reg()) | (1 << 7));
    if (Mmu->read_ram(HL.reg()) == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::SLA_hi(Register &Reg)
{
    if (Reg.hi()  & (1 << 7))
        AF.set_carryflag();
    else AF.clear_carryflag();
    Reg.set_hi(Reg.hi() << 1);
    if (Reg.hi() == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::SLA_lo(Register &Reg)
{
    if (Reg.lo()  & (1 << 7))
        AF.set_carryflag();
    else AF.clear_carryflag();
    Reg.set_lo(Reg.lo() << 1);
    if (Reg.lo() == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::SLA_HL()
{
    if (Mmu->read_ram(HL.reg())  & (1 << 7))
        AF.set_carryflag();
    else AF.clear_carryflag();
    Mmu->write_ram(HL.reg(), Mmu->read_ram(HL.reg()) << 1);
    if (Mmu->read_ram(HL.reg()) == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::SRA_hi(Register &Reg)
{
    if (Reg.hi()  & (1 << 0))
        AF.set_carryflag();
    else AF.clear_carryflag();
    Reg.set_hi(Reg.hi() >> 1);
    if (Reg.hi() == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::SRA_lo(Register &Reg)
{
    if (Reg.lo()  & (1 << 0))
        AF.set_carryflag();
    else AF.clear_carryflag();
    Reg.set_lo(Reg.lo() >> 1);
    if (Reg.lo() == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::SRA_HL()
{
    if (Mmu->read_ram(HL.reg())  & (1 << 0))
        AF.set_carryflag();
    else AF.clear_carryflag();
    Mmu->write_ram(HL.reg(), Mmu->read_ram(HL.reg()) >> 1);
    if (Mmu->read_ram(HL.reg()) == 0x0)
        AF.set_zflag();
    else AF.clear_zflag();
    AF.clear_halfcarryflag();
    AF.clear_subflag();
}

void Cpu::RES_hi(Register &Reg, int index)
{
    uint8_t tmp = Reg.hi();
    tmp &= ~(1<<index);
    Reg.set_hi(tmp);
}

void Cpu::RES_lo(Register &Reg, int index)
{
    uint8_t tmp = Reg.lo();
    tmp &= ~(1<<index);
    Reg.set_lo(tmp);
}

void Cpu::RES_HL(int index)
{
    uint8_t tmp = Mmu->read_ram(HL.reg());
    tmp &= ~(1<<index);
    Mmu->write_ram(HL.reg(), tmp);
}

void Cpu::run_thru(uint16_t min, uint16_t max)
{
    if (PC == min && !Mmu->get_boot_status())
        start_debug = true;
    if (PC == max && !Mmu->get_boot_status() && start_debug)
        exit(1);
}

uint16_t Cpu::get_PC()
{
    return PC;
}

void Cpu::set_PC(uint16_t value)
{
    PC = value;
}

int Cpu::timer_on()
{
	uint8_t control = Mmu->read_ram(0xFF07);
	return (control >> 2) & 1;
}

void Cpu::requestInterrupt(int id)
{
    uint8_t req = Mmu->read_ram(0xFF0F);
    req |= (1 << id);
    Mmu->write_ram(0xFF0F, req);
}

bool Cpu::get_interrupt_status()
{
   return enable_interrupt;
}

void Cpu::change_interrupt(bool value)
{
    enable_interrupt = value;
}
