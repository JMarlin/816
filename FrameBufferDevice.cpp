#include <stdlib.h>
#include "FrameBufferDevice.h"

FrameBufferDevice::FrameBufferDevice(word32 base) {
	
	this->_InitOk = false;

	this->AddResponseRange(base, base + 0x80000 - 1, RW_MASK_R | RW_MASK_W);
	this->_8bppVRAM = (byte*)malloc(640 * 480);

	for (int i = 0; i < 640 * 480; i++)
		this->_8bppVRAM[i] = (rand() & 0xFF);

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {

		printf("Couldn't init SDL: %s\n", SDL_GetError());

		return;
	}

	if ((this->_SDLWindow =
		SDL_CreateWindow(
			"816",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			640, 480,
			SDL_WINDOW_SHOWN
		)) == NULL
	) {

		printf("Couldn't create SDL window: %s\n", SDL_GetError());

		return;
	}

	if (
	    (this->_SDLRenderer =
	        SDL_CreateRenderer(this->_SDLWindow, -1, SDL_RENDERER_SOFTWARE)
		) == NULL
	) {

		printf("Couldn't create SDL renderer: %s\n", SDL_GetError());

		return;
	}

	if ((this->_FBSurface = 
		SDL_CreateRGBSurfaceFrom(
			this->_8bppVRAM, 
			640, 480, 
			8, 
			640, 
			0, 0, 0, 0)
		) == NULL
	) {

		printf("Couldn't create screen surface: %s\n", SDL_GetError());

		return;
	}

	SDL_Color colors[256];

	for (auto i = 0; i < 256; i++) {
		colors[i].r = ((i & 0x01) ? 0xFF : 0);
		colors[i].r -= colors[i].r == 0 ? 0 : ((i & 0x08) ? 0 : 0x80);
		colors[i].g = ((i & 0x02) ? 0xFF : 0);
		colors[i].g -= colors[i].g == 0 ? 0 : ((i & 0x08) ? 0 : 0x80);
		colors[i].b = ((i & 0x04) ? 0xFF : 0);
		colors[i].b -= colors[i].b == 0 ? 0 : ((i & 0x08) ? 0 : 0x80);
		colors[i].a = 0xFF;
	}

	SDL_Palette* palette = SDL_AllocPalette(256);

	SDL_SetPaletteColors(palette, colors, 0, 256);
	SDL_SetSurfacePalette(this->_FBSurface, palette);

	this->_FBRect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
	this->_FBRect->x = 0;
	this->_FBRect->y = 0;
	this->_FBRect->w = 640;
	this->_FBRect->h = 480;

	this->_LastTimestamp = 0;
	this->_InitOk = true;
}

FrameBufferDevice::~FrameBufferDevice() {
	
	free(this->_8bppVRAM);
	SDL_DestroyRenderer(this->_SDLRenderer);
	SDL_DestroyWindow(this->_SDLWindow);
	SDL_Quit();
}

bool FrameBufferDevice::_InternalReadByte(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* range, byte &b) {

	b = this->_InitOk ?
		((this->_8bppVRAM[(address - range->Start()) * 2] & 0x0F) << 4) | 
		(this->_8bppVRAM[((address - range->Start()) * 2) + 1] & 0x0F) :
		0xFF;

	return true;
}

bool FrameBufferDevice::_InternalWriteByte(word32 address, word32 timestamp, ResponseRange* range, byte b) {

	if (this->_InitOk) {

		this->_8bppVRAM[(address - range->Start()) * 2] = ((b & 0xF0) >> 4);
		this->_8bppVRAM[((address - range->Start()) * 2) + 1] = (b & 0x0F);
	}

	return true;
}

bool FrameBufferDevice::Refresh(word32 timestamp) {

	SDL_Event e;
	SDL_Surface* surf = this->_FBSurface;

	while (SDL_PollEvent(&e) != 0) {

		//if(e.type = SDL_QUIT) //TODO
	}

	//Baaaad, baaaad execution speed derived timing
	if (timestamp - this->_LastTimestamp >= 387 ||
		this->_LastTimestamp == 0) {

		SDL_BlitSurface(
			this->_FBSurface, NULL,
			SDL_GetWindowSurface(this->_SDLWindow), NULL
		);

		SDL_UpdateWindowSurface(this->_SDLWindow);

		this->_LastTimestamp = timestamp;
	}

	return false;
}