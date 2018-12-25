#pragma once

#include "device.h"
#include <list>

class DeviceAggregator : public Device {

public:
	DeviceAggregator();
	~DeviceAggregator();

	virtual bool GetInitOk();
	bool Refresh(word32 timestamp);
	bool TryReadByte(word32 address, word32 timestamp, word32 emulFlags, byte &b);
	bool TryWriteByte(word32 address, word32 timestamp, byte b);

protected:
	void _InstallDevice(Device* device);

	virtual bool _SelfRefresh(word32 timestamp) = 0;

	std::list<Device*> *_AggregatedDevices;
};
