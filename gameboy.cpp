#include "gameboy.h"

GameBoy::GameBoy()
{
    cpu = Cpu(mmu);
    gpu = Gpu(mmu);
    timer = Timer(mmu);
    cycle_counter = 0;
    SDL_Init(SDL_INIT_VIDEO);
    tile_map = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, VRAM_WIDTH * PX, VRAM_HEIGHT * PX, 32, 0,0,0,0);
    bg_map = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, BG_MAP_WIDTH * PX, BG_MAP_HEIGTH * PX, 32, 0,0,0,0);
    window = SDL_CreateRGBSurface(SDL_HWSURFACE | SDL_DOUBLEBUF, WIDTH * PX, HEIGHT * PX, 32, 0,0,0,0);
    screen = SDL_SetVideoMode((WIDTH + BG_MAP_WIDTH * 0) * PX,(HEIGHT + VRAM_HEIGHT * 0) * PX, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE);

    tile_map_pos.x = 0;
    tile_map_pos.y = HEIGHT * PX;

    joystate = 0x2F;
}

GameBoy::~GameBoy()
{

}

void GameBoy::step()
{
    int cycle = cpu.cpuStep();
    cycle_counter += cycle;
    timer.update_timer(cycle);
    gpu.gpuStep(cycle, window);
     //handleJoypad();
    handleInterrupts();
}

void GameBoy::boot()
{

    bool exit = false;

    while (!exit)
    {
        cycle_counter = 0;
        while (cycle_counter <= 69905)
            step();
        if (gpu.get_draw())
        {
            gpu.draw_pixels(window);
              if (mmu.read_ram(0xFF40) & (1<<1))
		    gpu.render_sprites(window);
            gpu.unset_draw();
            SDL_BlitSurface(window, NULL, screen, NULL);
            SDL_Flip(screen);
        }
    }
}

void GameBoy::handleInterrupts()
{
    if (cpu.get_interrupt_status())
    {
        uint8_t req = mmu.read_ram(0xFF0F);
        uint8_t enable = mmu.read_ram(0xFFFF);
        if (req > 0)
        {
            for (int i=0; i<5; i++)
            {
                if (req & (1 << i))
                    if (enable & (1 << i))
                    {
                        cout << "Interrupt " << i << " serviced at PC = " << hex << (int)cpu.get_PC();
                        cpu.executeOpcode();
                        serviceInterrupt(i);
                    }
            }
        }
    }
}

void GameBoy::serviceInterrupt(int interrupt)
{
    cpu.change_interrupt(false);
    uint8_t req = mmu.read_ram(0xFF0F);
    req &= ~(1 << interrupt);
    mmu.write_ram(0xFF0F, req);
    uint16_t tmp = cpu.get_PC();
    cpu.push_SP(tmp);
    switch (interrupt)
    {
        case 0:
            cpu.set_PC(0x0040);
            break;
        case 1:
            cpu.set_PC(0x0048);
            break;
        case 2:
            cpu.set_PC(0x0050);
            break;
        case 4:
            cpu.set_PC(0x0060);
            break;
    }
}

void GameBoy::handleJoypad()
{
    joystate = mmu.get_0xFF00();
    SDL_Event event;
    SDL_PollEvent(&event);
    if (event.type == SDL_KEYDOWN)
    {
        int key = -1;
        switch (event.key.keysym.sym)
        {
            case SDLK_a :
                key = 4 ;
                break ;
            case SDLK_s :
                key = 5 ;
                break ;
            case SDLK_RETURN :
                key = 7 ;
                break ;
            case SDLK_SPACE :
                key = 6;
                break ;
            case SDLK_RIGHT :
                key = 0 ;
                break ;
            case SDLK_LEFT :
                key = 1 ;
                break ;
            case SDLK_UP :
                key = 2 ;
                break ;
            case SDLK_DOWN :
                key = 3 ;
                break ;
        }
        if (key != -1)
        {
            setKeypress(key);
            mmu.setJoypad(joystate);
        }
    }
    else if (event.type == SDLK_UP)
    {
        int key = -1;
        switch (event.key.keysym.sym)
        {
            case SDLK_a :
                key = 4 ;
                break ;
            case SDLK_s :
                key = 5 ;
                break ;
            case SDLK_RETURN :
                key = 7 ;
                break ;
            case SDLK_SPACE :
                key = 6;
                break ;
            case SDLK_RIGHT :
                key = 0 ;
                break ;
            case SDLK_LEFT :
                key = 1 ;
                break ;
            case SDLK_UP :
                key = 2 ;
                break ;
            case SDLK_DOWN :
                key = 3 ;
                break ;
        }
        if (key != -1)
        {
            setKeyrelease(key);
            mmu.setJoypad(joystate);
        }
    }
}

void GameBoy::setKeypress(int key)
{
    bool previouslyUnset = false ;
    if (!(joystate & (1<<key)))
        previouslyUnset = true ;

    joystate &= ~(1<<key) ;
    bool button = true ;
    if (key > 3)
        button = true ;
    else
        button = false ;
    uint8_t keyReq = mmu.get_0xFF00();
    bool requestInterupt = false ;

    if (button && (!(keyReq & (1<<5))))
        requestInterupt = true ;
    else if (!button && (!(keyReq & (1<<4))))
        requestInterupt = true ;
    if (requestInterupt && !previouslyUnset)
        requestInterrupt(4);
}

void GameBoy::setKeyrelease(int key)
{
    joystate |= (1<<key) ;
}

void GameBoy::requestInterrupt(int id)
{
    uint8_t req = mmu.read_ram(0xFF0F);
    req |= (1 << id);
    mmu.write_ram(0xFF0F, req);
}

