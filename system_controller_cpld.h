#pragma once

#include <list>
#include "cpu.h"
#include "device_aggregator.h"
#include "ram_device.h"
#include "rom_device.h"
#include "XR88C681.h"

using namespace std;

class SystemControllerCPLD : public DeviceAggregator {

public:
	SystemControllerCPLD(char*);

private:
	bool _InternalReadByte(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool _InternalWriteByte(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
    bool _SelfRefresh(word32 timestamp);

	RAMDevice* _BaseRAMA;
	RAMDevice* _BaseRAMB;
	ROMDevice* _BootROM;
	XR88C681* _XR88C681;
};