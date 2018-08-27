#include <stdlib.h>
#include "ram_device.h"

RAMDevice::RAMDevice(word32 base, word32 size) {

	this->_StorageArea = (byte*)malloc(size);
	this->AddResponseRange(base, base + size - 1, RW_MASK_R | RW_MASK_W);
}

RAMDevice::~RAMDevice() {

	free(this->_StorageArea);
}

bool RAMDevice::_InternalReadByte(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* range, byte &b) {

	b = this->_StorageArea[address - range->Start()];

	return true;
}

bool RAMDevice::_InternalWriteByte(word32 address, word32 timestamp, ResponseRange* range, byte b) {

	this->_StorageArea[address - range->Start()] = b;

	return true;
}

bool RAMDevice::Refresh(word32 timestamp) {
	//Nothing to refresh
	return false;
}