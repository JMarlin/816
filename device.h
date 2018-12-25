#pragma once

#include "cpu.h"
#include "ResponseRange.h"
#include <list>

class Device {

public:

	Device();
	~Device();

	virtual bool Refresh(word32 timestamp) = 0;
	
	virtual bool GetInitOk();
	virtual bool TryReadByte(word32 address, word32 timestamp, word32 emulFlags, byte &b);
	virtual bool TryWriteByte(word32 address, word32 timestamp, byte b);
	ResponseRange* AddResponseRange(word32 start, word32 end, byte rw_mask);
	ResponseRange* AddResponseRange(word32 start, word32 end, byte rw_mask, ResponseRange::ReadCallback read_callback, ResponseRange::WriteCallback write_callback);

protected:
	virtual bool _InternalReadByte(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) = 0;
	virtual bool _InternalWriteByte(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) = 0;

	std::list<ResponseRange*> *_ResponseRanges;
	bool _InitOk;

private:
	ResponseRange* _MatchRange(word32 address, bool write);
};