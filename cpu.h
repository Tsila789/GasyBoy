#ifndef CPU_H
#define CPU_H

#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <vector>
#include <fstream>
#include <bitset>

#include "register.h"
#include "mmu.h"
#include "gpu.h"

using namespace std;

class Cpu
{
	private:
		MMU *Mmu;
	    SpecialRegister AF;
	    Register BC, DE, HL;
	    uint16_t PC, prev_CP;
	    uint16_t SP[0xFF];
	    uint8_t currentOpcode;
	    uint8_t bios_memory[0xFF];
	    uint8_t Memory[0xFFFF];
	    int cycle, SP_indice, divide_counter, timer_counter;
	    bool start_debug, gpu_debug;
	    bool enable_interrupt;
	public:
		Cpu();
	    Cpu(MMU &Mmu);
	    ~Cpu();
	    int cpuStep();
	    void fetchOpcode();
	    void executeOpcode();
	    void loadRom(char *filename);
	    int get_cycle();
	    void debug();
	    void run_thru(uint16_t min, uint16_t max = 0xFFFF);
	    int timer_on();
	    void requestInterrupt(int id);
	    bool get_interrupt_status();
	    void change_interrupt(bool value);
	    void POP_reg(uint16_t &value);



	    /*----- UTILITY ------*/

		uint16_t get_next_2byte();
		uint16_t get_next_2byte(uint16_t value);
		uint8_t modify_bit(uint8_t n, uint8_t pos, uint8_t value);
		uint16_t get_PC();
		void set_PC(uint16_t value);

	    void XOR_A(uint8_t value);

	    void OR_A(uint8_t value);

	    void BIT(uint8_t byte, int i);

	    void INC_reg_lo(Register &reg);
	    void INC_reg_hi(Register &reg);
	    void INC_reg(Register &Reg);
	    void INC_Mem(uint16_t value);

	    void DEC_reg_lo(Register &reg);
	    void DEC_reg_hi(Register &reg);
	    void DEC_reg(Register &reg);
	    void DEC_Mem(uint16_t value);

	    void add2bytes_to_memory(uint16_t pos, uint16_t value);
	    void pop_SP();
	    void push_SP(uint16_t value);
	    bool get_gpu_debug();

	    void SUB(uint8_t value);

	    void CP(uint8_t value);

	    void LD_A(uint8_t value);
	    void LD_reg(Register &Reg, uint16_t value);
		void LD_reg_hi(Register &Reg, uint8_t value);
		void LD_reg_lo(Register &Reg, uint8_t value);
	    void LD_Mem(uint16_t adrr, uint8_t value);
	    void LDH_A(uint16_t value);
	    void LDH_Mem(uint8_t value);

	    void ADD_A(uint8_t value);
	    void ADD_HL(uint16_t value);
	    void ADD_SP(uint8_t value);
	    void ADC_A(uint8_t value);

	    void JP(uint16_t value);
	    void JP_NZ();
	    void JP_Z();
	    void JP_NC();
	    void JP_C();

	    void RL_n(uint16_t adrr);
	    void RL_reg_hi(Register &Reg);
		void RL_reg_lo(Register &Reg);

		void RR_n(uint16_t adrr);
	    void RR_reg_hi(Register &Reg);
		void RR_reg_lo(Register &Reg);

		void RL_A();

		void SBC_A(uint8_t value);

		void SRL_HL();
		void SRL_reg_hi(Register &Reg);
		void SRL_reg_lo(Register &Reg);

		void AND_A(uint8_t value);

		void RST(uint8_t value);

		void POP_reg(Register &Reg);

		void RET();
		void RET_Z();
		void RET_NZ();
		void RET_C();
		void RET_NC();
		void RETI();

		void JR_C(uint8_t value);
		void JR_NC(uint8_t value);
		void JR_Z(uint8_t value);
		void JR_NZ(uint8_t value);

		void SWAP_reg_lo(Register &Reg);
		void SWAP_reg_hi(Register &Reg);
		void SWAP_HL();

		void PUSH(Register &Reg);

		void CALL_NZ();
		void CALL_Z();
		void CALL_NC();
		void CALL_C();
		void CALL(uint16_t value);

		void RLCA();
		void RRCA();
		void CPL();
		void SCF();
		void CCF();

		void EI();
		void DI();
		//MOST 0xCB INSTRUCTION FUNCTION

		void SET_HL(int index);
		void SET_hi(Register &Reg, int index);
		void SET_lo(Register &Reg, int index);

		void RLC_hi(Register &Reg);
		void RLC_lo(Register &Reg);
		void RLC_HL();

		void RRC_hi(Register &Reg);
		void RRC_lo(Register &Reg);
		void RRC_HL();

		void SLA_hi(Register &Reg);
		void SLA_lo(Register &Reg);
		void SLA_HL();

		void SRA_hi(Register &Reg);
		void SRA_lo(Register &Reg);
		void SRA_HL();

		void RES_hi(Register &Reg, int index);
		void RES_lo(Register &Reg, int index);
		void RES_HL(int index);
};

#endif
