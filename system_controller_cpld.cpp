#include "system_controller_cpld.h"


SystemControllerCPLD::SystemControllerCPLD(char* rom_path) {

	this->_BaseRAM = new RAMDevice(0x0000, 0x7FF0);
	this->_BootROM = new ROMDevice(rom_path, 0x8000, 0x8000);
	this->_XR88C681 = new XR88C681(0x7FF0);
	this->_FrameBuffer = new FrameBufferDevice(0xF00000);
	this->_InstallDevice(this->_BaseRAM);
	this->_InstallDevice(this->_BootROM);
	this->_InstallDevice(this->_XR88C681);
	this->_InstallDevice(this->_FrameBuffer);
	this->_InitOk = true;
}


bool SystemControllerCPLD::_InternalReadByte(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	//This guy doesn't really have any registers to R/W
	return false;
}
bool SystemControllerCPLD::_InternalWriteByte(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	//This guy doesn't really have any registers to R/W
	return false;
}

bool SystemControllerCPLD::_SelfRefresh(word32 timestamp) {
	//No real internal state to keep up to date
	return false;
}

bool SystemControllerCPLD::StartSendFile(char* filename) {

	return this->_XR88C681->StartSendFile(filename);
}