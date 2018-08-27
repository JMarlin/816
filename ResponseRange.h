#pragma once

#include "cpu.h"

#define RW_MASK_R 0x01
#define RW_MASK_W 0x02
#define RW_VALID_MASKS (RW_MASK_R | RW_MASK_W)

class Device;

class ResponseRange {

public:
	typedef bool(*ReadCallback)(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	typedef bool(*WriteCallback)(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);

	ResponseRange(word32, word32, byte);
	ResponseRange(word32, word32, byte, ResponseRange::ReadCallback, ResponseRange::WriteCallback);

	word32 Start(void);
	word32 End(void);
	ResponseRange::ReadCallback GetReadCallback();
	ResponseRange::WriteCallback GetWriteCallback();
	bool RespondsToAddress(word32, bool);

private: 
	word32 _Start;
	word32 _End;
	byte _RWMask;
	ResponseRange::ReadCallback _ReadCallback;
	ResponseRange::WriteCallback _WriteCallback;
};
