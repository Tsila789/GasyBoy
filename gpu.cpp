#include "gpu.h"

Gpu::Gpu()
{

}

Gpu::Gpu(MMU &mmu)
{
    Mmu = &mmu;
    modeClock = 0;
    graphicMode = 0;
    line = 0;
    for (int j = 0; j < 512; j++)
        Tile[j] = SDL_CreateRGBSurface(SDL_HWSURFACE, 8 * PX, 8 * PX, 32, 0, 0, 0, 0);
    W = SDL_CreateRGBSurface(SDL_HWSURFACE, 8 * PX, 8 * PX, 32, 0, 0, 0, 0);
    B = SDL_CreateRGBSurface(SDL_HWSURFACE, 8 * PX, 8 * PX, 32, 0, 0, 0, 0);
    L = SDL_CreateRGBSurface(SDL_HWSURFACE, 8 * PX, 8 * PX, 32, 0, 0, 0, 0);
    D = SDL_CreateRGBSurface(SDL_HWSURFACE, 8 * PX, 8 * PX, 32, 0, 0, 0, 0);

    SDL_FillRect(W, NULL, SDL_MapRGB(W->format, 255,255,255));
    SDL_FillRect(B, NULL, SDL_MapRGB(W->format, 0,0,0));
    SDL_FillRect(L, NULL, SDL_MapRGB(W->format, 0xCC,0xCC,0xCC));
    SDL_FillRect(D, NULL, SDL_MapRGB(W->format, 0x77,0x77,0x77));

    draw = false;

    scanline_counter = 456;
    retraceLY = 456;
}

Gpu::~Gpu()
{

}

uint8_t Gpu::currScanline()
{
    return Mmu->read_ram(0xFF44);
}


void Gpu::show_tile_map(SDL_Surface * screen)
{
    for (int i = 0; i < 512; i++)
        show_tile(i, Tile[i]);
    int k = 0, l = 0;
    for (int j = 0; j < 512; j++)
    {
        if (j % 16 == 0 && j > 0)
        {
            l++;
            k=0;
        }
        SDL_Rect pos;
        pos.x = 8 * PX * k;
        k++;
        pos.y = PX * 8 * l;
        SDL_BlitSurface(Tile[j], NULL, screen, &pos);
    }
}

void Gpu::show_tile_line(uint8_t a, uint8_t b, SDL_Surface * screen, int line)
{
    for (int i = 7; i >= 0; i--)
    {
        SDL_Rect pos;
        pos.x = (7 - i) * PX;
        pos.y = line * PX;
        if (!(a & (1 << i)))
        {
            if (!(b & (1 << i)))
                SDL_BlitSurface(W, NULL, screen, &pos);
            else if ((b & (1 << i)))
                SDL_BlitSurface(L, NULL, screen, &pos);
        }
        else if ((a & (1 << i)))
        {
            if (!(b & (1 << i)))
                SDL_BlitSurface(D, NULL, screen, &pos);
            else if ((b & (1 << i)))
                SDL_BlitSurface(B, NULL, screen, &pos);
        }
    }
}

void Gpu::show_tile(int i, SDL_Surface * tile)
{
    show_tile_line(Mmu->get_vram()[16 * i + 1], Mmu->get_vram()[16 * i + 0], tile, 0);
    show_tile_line(Mmu->get_vram()[16 * i + 3], Mmu->get_vram()[16 * i + 2], tile, 1);
    show_tile_line(Mmu->get_vram()[16 * i + 5], Mmu->get_vram()[16 * i + 4], tile, 2);
    show_tile_line(Mmu->get_vram()[16 * i + 7], Mmu->get_vram()[16 * i + 6], tile, 3);
    show_tile_line(Mmu->get_vram()[16 * i + 9], Mmu->get_vram()[16 * i + 8], tile, 4);
    show_tile_line(Mmu->get_vram()[16 * i + 11], Mmu->get_vram()[16 * i + 10], tile, 5);
    show_tile_line(Mmu->get_vram()[16 * i + 13], Mmu->get_vram()[16 * i + 12], tile, 6);
    show_tile_line(Mmu->get_vram()[16 * i + 15], Mmu->get_vram()[16 * i + 14], tile, 7);

}

uint8_t Gpu::get_scx()
{
    return Mmu->read_ram(0xFF43);
}

uint8_t Gpu::get_scy()
{
    return Mmu->read_ram(0xFF42);
}

uint8_t Gpu::get_windowx()
{
    return Mmu->read_ram(0xFF4B);
}

uint8_t Gpu::get_windowy()
{
    return Mmu->read_ram(0xFF4A);
}

uint8_t Gpu::get_LCDC_status()
{
    return Mmu->read_ram(0xFF40);
}

void Gpu::render_tiles()
{
    if (Mmu->read_ram(0xFF40) & (1<<0))
    {
        uint16_t tileData = 0 ;
        uint16_t backgroundMemory =0 ;
        bool unsig = true ;

        uint8_t scrollY = Mmu->read_ram(0xFF42) ;
        uint8_t scrollX = Mmu->read_ram(0xFF43) ;
        uint8_t windowY = Mmu->read_ram(0xFF4A) ;
        uint8_t windowX = Mmu->read_ram(0xFF4B) - 7;

        bool usingWindow = false ;

        if (Mmu->read_ram(0xFF40) & (1<<5))
        {
            if (windowY <= Mmu->read_ram(0xFF44))
                usingWindow = true ;
        }
        else
        {
            usingWindow = false ;
        }
        if (Mmu->read_ram(0xFF40) & (1<<4))
        {
            tileData = 0x8000 ;
        }
        else
        {
            tileData = 0x8800 ;
            unsig= false ;
        }
        if (false == usingWindow)
        {
            if (Mmu->read_ram(0xFF40) & (1<<3))
                backgroundMemory = 0x9C00 ;
            else
                backgroundMemory = 0x9800 ;
        }
        else
        {
            if (Mmu->read_ram(0xFF40) & (1<<6))
                backgroundMemory = 0x9C00 ;
            else
                backgroundMemory = 0x9800 ;
        }


        uint8_t yPos = 0 ;

        if (!usingWindow)
            yPos = scrollY + Mmu->read_ram(0xFF44) ;
        else
            yPos = Mmu->read_ram(0xFF44) - windowY;

        uint16_t tileRow = (((uint8_t)(yPos/8))*32) ;

        for (int pixel = 0 ; pixel < 160; pixel++)
        {
            uint8_t xPos = pixel+scrollX ;

            if (usingWindow)
            {
                if (pixel >= windowX)
                {
                    xPos = pixel - windowX ;
                }
            }

            uint16_t tileCol = (xPos/8) ;
            int16_t tileNum ;

            if(unsig)
                tileNum = (uint8_t)Mmu->read_ram(backgroundMemory+tileRow + tileCol) ;
            else
                tileNum = (int8_t)Mmu->read_ram(backgroundMemory+tileRow + tileCol) ;

            uint16_t tileLocation = tileData ;

            if (unsig)
                tileLocation += (tileNum * 16) ;
            else
                tileLocation += ((tileNum+128) *16) ;

            uint8_t line = yPos % 8 ;
            line *= 2;
            uint8_t data1 = Mmu->read_ram(tileLocation + line) ;
            uint8_t data2 = Mmu->read_ram(tileLocation + line + 1) ;

            int colourBit = xPos % 8 ;
            colourBit -= 7 ;
            colourBit *= -1 ;

            int colourNum = BitGetVal(data2,colourBit) ;
            colourNum <<= 1;
            colourNum |= BitGetVal(data1,colourBit) ;

            int col = GetColour(colourNum, 0xFF47) ;
            int red = 0;
            int green = 0;
            int blue = 0;

            switch(col)
            {
                case WHITE:
                    red = 255;
                    green = 255 ;
                    blue = 255;
                    break ;
                case LIGHT:
                    red = 0xCC;
                    green = 0xCC ;
                    blue = 0xCC;
                    break ;
                case DARK:
                    red = 0x77;
                    green = 0x77 ;
                    blue = 0x77;
                    break ;
                case BLACK:
                    red = 0;
                    green = 0 ;
                    blue = 0;
                    break ;
            }

            int finaly = Mmu->read_ram(0xFF44) ;

            pixels[pixel][finaly][0] = red ;
            pixels[pixel][finaly][1] = green ;
            pixels[pixel][finaly][2] = blue ;
        }
    }
}


uint8_t Gpu::get_color(int id, uint16_t palette)
{
    uint8_t data = Mmu->read_ram(palette);
    int hi = 2 * id + 1, lo = 2 * id;
    int bit1 = (data >> hi)  & 1;
    int bit0 = (data >> lo)  & 1;
    return (bit1 << 1) | bit0;
}

void Gpu::draw_pixels(SDL_Surface *window)
{
    for (int i=0; i<144; i++)
    {
        for (int j=0; j<160; j++)
        {
            int red = pixels[j][i][0], blue = pixels[j][i][1], green = pixels[j][i][2];
            pixels_pos.x = j*PX;
            pixels_pos.y = i*PX;
            if (red == 0xFF && green == 0xFF && blue == 0xFF)
                SDL_BlitSurface(W, NULL, window, &pixels_pos);
            else if (red == 0xCC && green == 0xCC && blue == 0xCC)
                SDL_BlitSurface(D, NULL, window, &pixels_pos);
            else if (red == 0x77 && green == 0x77 && blue == 0x77)
                SDL_BlitSurface(L, NULL, window, &pixels_pos);
            else if (red == 0x0 && green == 0x0 && blue == 0x0)
                SDL_BlitSurface(B, NULL, window, &pixels_pos);
        }
    }
}

bool Gpu::get_draw()
{
    return draw;
}

void Gpu::unset_draw()
{
    draw = false;
}


void Gpu::requestInterrupt(int id)
{
    uint8_t req = Mmu->read_ram(0xFF0F);
    req |= (1 << id);
    Mmu->write_ram(0xFF0F, req);
}

int Gpu::BitGetVal(uint8_t data, int position)
{
    if (data & (1 << position))
        return 1;
    else
        return 0 ;
}


int Gpu::GetColour(uint8_t colourNum, uint16_t address)
{
    int res;
    uint8_t palette = Mmu->read_ram(address) ;
    int hi = 0 ;;
    int lo = 0 ;

    switch (colourNum)
    {
        case 0:
            hi = 1 ;
            lo = 0 ;
            break ;
        case 1:
            hi = 3 ;
            lo = 2 ;
            break ;
        case 2:
            hi = 5 ;
            lo = 4 ;
            break ;
        case 3:
            hi = 7 ;
            lo = 6 ;
            break ;
    }

    int colour = 0;
    colour = BitGetVal(palette, hi) << 1;
    colour |= BitGetVal(palette, lo) ;

    switch (colour)
    {
        case 0:
            res = WHITE ;
            break ;
        case 1:
            res = LIGHT ;
            break ;
        case 2:
            res = DARK ;
            break ;
        case 3:
            res = BLACK ;
            break ;
    }

    return res ;
}


void Gpu::render_sprites(SDL_Surface *window)
{
    if (Mmu->read_ram(0xFF40) & (1<<1))
    {
        for (int i=0; i<40; i+=4)
        {
            uint8_t yPos = Mmu->read_ram(0xFE00+i)- 16;
            uint8_t xPos = Mmu->read_ram(0xFE01+i)-8;
            uint8_t tileLocation = Mmu->read_ram(0xFE02+i) ;
            uint8_t attributes = Mmu->read_ram(0xFE03+i) ;

            bool yFlip = (Mmu->read_ram(attributes) & (1<<6)) ;
            bool xFlip = (Mmu->read_ram(attributes) & (1<<5)) ;
            show_tile(tileLocation,Tile[tileLocation]);
            SDL_Rect pos;
            pos.x = xPos * PX;
            pos.y = yPos * PX;
            SDL_BlitSurface(Tile[tileLocation],NULL,window,&pos);
        }
    }
}

void Gpu::gpuStep(int cycles, SDL_Surface *window)
{
    setLCDStatus();
    if (Mmu->read_ram(0xFF40) & (1<<7))
        scanline_counter -= cycles;
    else
        return;
    if (scanline_counter <= 0)
    {
        Mmu->direct_set(0xFF44, Mmu->read_ram(0xFF44) + 1);
        uint8_t current_line = Mmu->read_ram(0xFF44);
        scanline_counter = 456;
        if (current_line == 144)
            requestInterrupt(0);
        else if (current_line > 153)
            Mmu->direct_set(0xFF44, 0);
        else if (current_line < 144)
            draw_scanline(window);
    }
}

/*void Gpu::draw_currentline(SDL_Surface *window)
{
    if (!(Mmu->read_ram(0xFF40) & (1<<7)))
		return ;

	Mmu->direct_set(0xFF44, Mmu->read_ram(0xFF44) + 1);

	retraceLY = 456 ;

	uint8_t scanLine = Mmu->read_ram(0xFF44);

	if ( scanLine == 0x90)
		requestInterrupt(0);

	if (scanLine > 0x99)
		Mmu->direct_set(0xFF44,0);

	if (scanLine < 0x90)
		draw_scanline(window);
}*/

void Gpu::draw_scanline(SDL_Surface *window)
{
    if (Mmu->read_ram(0xFF40) & (1<<0))
        render_tiles();
    draw = true;
}


void Gpu::setLCDStatus()
{
    uint8_t lcdStatus = Mmu->read_ram(0xFF41);

    if (!(Mmu->read_ram(0xFF40) & (1<<7)))
    {
        scanline_counter = 456 ;
        Mmu->direct_set(0xFF44, 0);
        lcdStatus &= 252 ;
        lcdStatus |= (1<<0);
        Mmu->write_ram(0xFF41, lcdStatus);
        return ;
    }
    uint8_t lY = Mmu->read_ram(0xFF44);
    uint8_t currentMode = Mmu->read_ram(0xFF41);
    int mode = 0 ;
    bool reqInt = false ;
    if (lY >= 144)
    {
        mode = 1 ;
        lcdStatus |= (1<<0);
        lcdStatus &= ~(1<<1);
        reqInt = (lcdStatus & (1<<4));
    }
    else
    {
        int mode2Bounds = (456 - 80) ;
        int mode3Bounds = (mode2Bounds - 172) ;
        if (scanline_counter >= mode2Bounds)
        {
            mode = 2 ;
            lcdStatus |= (1<<1);
            lcdStatus &= ~(1<<0);
            reqInt = (lcdStatus & (1<<5));
        }
        else if (scanline_counter >= mode3Bounds)
        {
            mode = 3 ;
            lcdStatus |= (1<<1);
            lcdStatus |= (1<<0);
        }
        else
        {
            mode = 0 ;
            lcdStatus &= ~(1<<1);
            lcdStatus &= ~(1<<0);
            reqInt = (lcdStatus & (1<<3));
        }
    }
    if (reqInt && (currentMode != mode))
        requestInterrupt(1);
    if (lY == Mmu->read_ram(0xFF45))
    {
        lcdStatus |= (1<<2);
        if (lcdStatus & (1<<6))
            requestInterrupt(1);
    }
    else
        lcdStatus &= ~(1<<2) ;

    Mmu->write_ram(0xFF41, lcdStatus) ;
}
