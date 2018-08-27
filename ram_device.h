#pragma once

#include "device.h"

class RAMDevice : public Device {

public:
	RAMDevice(word32 base, word32 size);
	~RAMDevice();

	bool Refresh(word32 timestamp);

protected:
	bool _InternalReadByte(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool _InternalWriteByte(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);

private:
	byte* _StorageArea;
};
