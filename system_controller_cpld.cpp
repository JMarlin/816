#include "system_controller_cpld.h"


SystemControllerCPLD::SystemControllerCPLD(char* rom_path) {

	this->_BaseRAMA = new RAMDevice(0x0000, 0x7FF0); //Make sure that, given the base and the size, the device sets up its own response range
	this->_BootROM = new ROMDevice(rom_path, 0x8000, 0x8000); //Make sure that, given the base and the size, the device sets up its own response range
	this->_XR88C681 = new XR88C681(0x7FF0);
	this->_InstallDevice(this->_BaseRAMA);
	this->_InstallDevice(this->_BootROM);
	this->_InstallDevice(this->_XR88C681);
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
