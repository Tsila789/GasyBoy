#ifndef GAMEBOY_H
#define GAMEBOY_H

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "mmu.h"
#include "cpu.h"
#include "gpu.h"
#include "defs.h"
#include "timer.h"



class GameBoy
{
    private:
        MMU mmu;
        Cpu cpu;
        Gpu gpu;
        int cycle_counter;
        SDL_Surface *screen, *window, *tile_map, *bg_map;
        SDL_Rect tile_map_pos, bg_map_pos, window_pos;
        Timer timer;
        uint8_t joystate;
    public:
        GameBoy();
        ~GameBoy();
        void boot();
        void step();
        void handleJoypad();
        void handleInterrupts();
        void requestInterrupt(int id);
        void serviceInterrupt(int id);
        void setKeypress(int key);
        void setKeyrelease(int key);
};

#endif // GAMEBOY_H
