#pragma once

#include "device.h"
#include <SDL.h>

class FrameBufferDevice : public Device {

public:
	FrameBufferDevice(word32 base);
	~FrameBufferDevice();

	bool Refresh(word32 timestamp);

protected:
	bool _InternalReadByte(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool _InternalWriteByte(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);

private:
	 SDL_Window* _SDLWindow;
	 SDL_Renderer* _SDLRenderer;
	 byte* _8bppVRAM;
	 SDL_Surface* _FBSurface;
	 SDL_Rect* _FBRect;
	 word32 _LastTimestamp;
};
