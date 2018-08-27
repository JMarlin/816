#include <stdlib.h>
#include <stdio.h>
#include "rom_device.h"

ROMDevice::ROMDevice(char* image_path, word32 base, word32 size) {

	auto image_file = fopen(image_path, "rb");

	if(!image_file)
		return; //TODO: Report an actual error status

	this->_StorageArea = (byte*)malloc(size);
	fread_s(this->_StorageArea, size, 1, size, image_file);
	fclose(image_file);
	this->AddResponseRange(base, base + size - 1, RW_MASK_R);	
}

ROMDevice::~ROMDevice() {

	free(this->_StorageArea);
}

bool ROMDevice::_InternalReadByte(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* range, byte &b) {

	b = this->_StorageArea[address - range->Start()];

	return true;
}

bool ROMDevice::_InternalWriteByte(word32 address, word32 timestamp, ResponseRange* range, byte b) {

	//We didn't register to respond to a writable area, so this should actually
	//never get called
	return false;
}

bool ROMDevice::Refresh(word32 timestamp) {
	//Nothing to refresh
	return false;
}