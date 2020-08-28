#include "mmu.h"

MMU::MMU()
{
	srand(time(0));
	for (int i = 0; i < 0x8000; i++)
		ROM[i] = 0x0;
	for (int i = 0; i < 0x2000; i++)
		VRAM[i] = 0;
	for (int i = 0; i < 0x2000; i++)
		ExtRAM[i] = 0x0;
	for (int i = 0; i < 0x4000; i++)
		WorkingRAM[i] = 0x0;
	in_bios = true;
	modified_vram = false;
	char filename[50] = "./TETRIS.gb";
	loadRom(filename);
	WorkingRAM[0xFF00 - 0xC000] = 0xEF;
	WorkingRAM[0xFF85 - 0xC000] = 0x01;

	joypadState = 0x0F;
    for (int i=0; i<8; i++)
        keys[i] = false;
}

MMU::~MMU()
{
	for (int i = 0; i < 0x8000; i++)
		ROM[i] = 0x0;
	for (int i = 0; i < 0x2000; i++)
		VRAM[i] = 0x0;
	for (int i = 0; i < 0x2000; i++)
		ExtRAM[i] = 0x0;
	for (int i = 0; i < 0x4000; i++)
		WorkingRAM[i] = 0x0;
	in_bios = true;
}

void MMU::disable_bios()
{
	in_bios = false;
}

bool MMU::get_boot_status()
{
	return in_bios;
}

uint8_t MMU::read_ram(uint16_t adrr)
{
	if (adrr < 0x100)
	{
		if (in_bios)
			return Bios[adrr];
		else return ROM[adrr];
	}
	else if (adrr >= 0x100)
	{
		if (adrr < 0x8000)
			return ROM[adrr];
		else if (adrr >= 0x8000 && adrr < 0xA000)
			return VRAM[adrr - 0x8000];
		else if (adrr >= 0xA000 && adrr < 0xC000)
			return ExtRAM[adrr - 0xA000];
		else if (adrr >= 0xC000 && adrr <= 0xFFFF)
		{
			if (adrr == 0xFF00)
				return 0b00010111;//GetJoyPad();
			else return WorkingRAM[adrr - 0xC000];
		}
	}
}

void MMU::write_ram(uint16_t adrr, uint8_t value)
{
	if (adrr < 0x7FFF)
		return;
	if (adrr >= 0x8000 && adrr < 0xA000)
	{
		VRAM[adrr - 0x8000] = value;
		if (value != 0)
			modified_vram = true;
	}
	else if (adrr >= 0xA000 && adrr < 0xC000)
		ExtRAM[adrr - 0xA000] = value;
	else if (adrr >= 0xC000 && adrr <= 0xFFFF)
	{
		if (adrr == 0xFF46)
 			DoDMATransfert(value);
		if (adrr == 0xFF44)
			WorkingRAM[adrr] = 0;
		if (adrr != 0xFF85)// && adrr != 0xFF00)
			WorkingRAM[adrr - 0xC000] = value;
		else return;
	}
}

void MMU::set_pad(uint8_t value)
{
    if (!(value & (1<<5)))
        joypadState = 0x1F;//0b00011111;
    else if (!(value & (1<<4)))
        joypadState = 0x2F;//0b00101111;
}

uint8_t MMU::GetJoyPad()
{
    set_pad(get_0xFF00());
    SDL_Event event;
    SDL_PollEvent(&event);
    if (event.type == SDL_KEYDOWN)
    {
        requestInterrupt(4);
        switch (event.key.keysym.sym)
        {
            case SDLK_a :
                keys[0] = true ;
                break ;
            case SDLK_z :
                keys[1] = true;
                break ;
            case SDLK_q :
                keys[2] = true;
                break ;
            case SDLK_s :
                keys[3] = true;
                break ;
            case SDLK_RIGHT :
                keys[4] = true ;
                break ;
            case SDLK_LEFT :
                keys[5] = true;
                break ;
            case SDLK_UP :
                keys[6] = true;
                break ;
            case SDLK_DOWN :
                keys[7] = true;
                break ;
        }
    }
    else if (event.type == SDL_KEYUP)
    {
        switch (event.key.keysym.sym)
        {
            case SDLK_a :
                keys[0] = false ;
                break ;
            case SDLK_z :
                keys[1] = false;
                break ;
            case SDLK_q :
                keys[2] = false;
                break ;
            case SDLK_s :
                keys[3] = false;
                break ;
            case SDLK_RIGHT :
                keys[4] = false ;
                break ;
            case SDLK_LEFT :
                keys[5] = false;
                break ;
            case SDLK_UP :
                keys[6] = false;
                break ;
            case SDLK_DOWN :
                keys[7] = false;
                break ;
        }
    }
    if (!(joypadState & (1<<5)))
    {
        if (keys[4])
        {
            joypadState &= ~(1<<0);
        }
        else
        {
            joypadState |= (1<<0);
        }
        if (keys[5])
        {
            joypadState &= ~(1<<1);
        }
        else
        {
            joypadState |= (1<<1);
        }
        if (keys[6])
        {
            joypadState &= ~(1<<2);
        }
        else
        {
            joypadState |= (1<<2);
        }
        if (keys[7])
        {
            joypadState &= ~(1<<3);
        }
        else
        {
            joypadState |= (1<<3);
        }
        return joypadState;
    }
    else if (!(joypadState & (1<<4)))
    {
        if (keys[4])
        {
            joypadState &= ~(1<<0);
        }
        else
        {
            joypadState |= (1<<0);
        }
        if (keys[5])
        {
            joypadState &= ~(1<<1);
        }
        else
        {
            joypadState |= (1<<1);
        }
        if (keys[6])
        {
            joypadState &= ~(1<<2);
        }
        else
        {
            joypadState |= (1<<2);
        }
        if (keys[7])
        {
            joypadState &= ~(1<<3);
        }
        else
        {
            joypadState |= (1<<3);
        }
        return joypadState;
    }
}

void MMU::loadRom(char *filename)
{
	ifstream Rom(filename, ios::in | ios::binary | ios::ate);
	if (Rom.is_open())
	{
		int RomSize = Rom.tellg();
		char *buff = new char[RomSize];
		Rom.seekg(0, std::ios::beg);
		Rom.read(buff, RomSize);
		for (int i = 0; i < RomSize; i++)
			ROM[i] = (uint8_t) buff[i];

		Rom.close();
	}
}

uint8_t *MMU::get_vram()
{
	return VRAM;
}

uint8_t *MMU::get_rom()
{
	return ROM;
}

uint8_t *MMU::get_extram()
{
	return ExtRAM;
}

uint8_t *MMU::get_workingram()
{
	return WorkingRAM;
}

void MMU::out_vram()
{
	ofstream Dump("./dump_vram.bin", ios::out | ios::binary);
	char *dump_buff = new char[0x2000];
	for (int i = (int)0x0; i < (int)0x2000; i++)
		dump_buff[i] = (char)get_vram()[i];
	Dump.write(dump_buff, 0x2000);
	Dump.close();
}

bool MMU::get_vram_state()
{
	return modified_vram;
}

void MMU::set_vram_state(bool value)
{
	modified_vram = value;
}

uint8_t **MMU::get_bg_array()
{
	uint8_t **matrix = new uint8_t *[256];
	for (int i = 0; i < 256; i++)
		matrix[i] = new uint8_t[256];
	uint16_t baseAdress;
	if (get_LCDC_BGTileMap())
		baseAdress = 0x9C00;
	else
		baseAdress = 0x9800;

	for (int i = 0; i < 1024; i++)
	{
		uint8_t currentTile = read_ram(baseAdress + i);
		uint16_t tileAdress;
		if (get_LCDC_BGWindowTile())
			tileAdress = 0x8000 + (currentTile * 0x10);
		else
		{
			if (currentTile <= 127)
				tileAdress = 0x9000 + (currentTile * 0x10);
			else
				tileAdress = 0x8800 + (currentTile * 0x10);
		}
		for (int j = 0; j < 8; j++)
		{
			for (int k = 0; k < 8; k++)
			{
				int x = 0, y = 0;
				uint8_t pixel = (read_ram(tileAdress + (j * 2)) >> (7 - k)) & 1;
				pixel = pixel << 1;
				pixel = pixel | ((read_ram(tileAdress + 1 + (j * 2)) >> (7 - k)) & 1);
				matrix[j + ((i / 32) * 8)][k + ((i % 32) * 8)] = get_paletteColor(pixel);
			}
		}
	}
	return matrix;
}

bool MMU::get_LCDC_BGTileMap()
{
	return (read_ram(0xFF40) & 0b00001000) == 0b00001000;
}

bool MMU::get_LCDC_BGWindowTile()
{
	return (read_ram(0xFF40) & 0b00010000) == 0b00010000;
}

uint8_t MMU::get_paletteColor(uint8_t index)
{
	uint8_t palette[4] = { 0 };
	palette[0] = read_ram(0xFF47) & 0b00000011;
	palette[1] = (read_ram(0xFF47) & 0b0000110) >> 2;
	palette[2] = (read_ram(0xFF47) & 0b00110000) >> 4;
	palette[3] = (read_ram(0xFF47) & 0b11000000) >> 6;
	return palette[index];
}

void MMU::direct_set(uint16_t adrr, uint8_t value)
{
	WorkingRAM[adrr - 0xC000] = value;
}


void MMU::DoDMATransfert(uint8_t value)
{
	uint16_t data = (value << 8);
	for (int i=0; i<0xA0; i++)
	{
		uint8_t tmp = read_ram(data + i);
		direct_set(0xFE00 + i, tmp);
	}

	/*ofstream Dump("./oam.bin", ios::out | ios::binary);
	for (uint8_t i = 0; i < 0xA0; i++)
		Dump <<  WorkingRAM[0xFE00 + (i >> 8)];
	Dump.close();*/
}

uint8_t MMU::get_0xFF00()
{
	return WorkingRAM[0xFF00 - 0xC000];
}

void MMU::setJoypad(uint8_t value)
{
	joypadState = value;
}

void MMU::requestInterrupt(int id)
{
    uint8_t req = read_ram(0xFF0F);
    req |= (1 << id);
   write_ram(0xFF0F, req);
}


void MMU::setKeypress(int key)
{
    bool previouslyUnset = false ;
    if (!(joypadState & (1<<key)))
        previouslyUnset = true ;

    joypadState &= ~(1<<key) ;
    bool button = true ;
    if (key > 3)
        button = true ;
    else
        button = false ;
    uint8_t keyReq =get_0xFF00();
    bool requestInterupt = false ;

    if (button && (!(keyReq & (1<<5))))
        requestInterupt = true ;
    else if (!button && (!(keyReq & (1<<4))))
        requestInterupt = true ;
    if (requestInterupt && !previouslyUnset)
        requestInterrupt(4);
}

void MMU::setKeyrelease(int key)
{
    joypadState |= (1<<key) ;
}
