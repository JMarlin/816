#include "device_aggregator.h"

DeviceAggregator::DeviceAggregator() {
	
	this->_AggregatedDevices = new std::list<Device*>();
}

DeviceAggregator::~DeviceAggregator() {

	for(auto iter = this->_AggregatedDevices->begin(); iter != this->_AggregatedDevices->end(); iter++)
		delete *iter;

	delete this->_AggregatedDevices;
}

bool DeviceAggregator::GetInitOk() {

	if (!this->_InitOk)
		return false;

	for (auto iter = this->_AggregatedDevices->begin(); iter != this->_AggregatedDevices->end(); iter++)
		if (!(*iter)->GetInitOk())
			return false;

	return true;
}

void DeviceAggregator::_InstallDevice(Device* device) {
	
	this->_AggregatedDevices->push_back(device);
}

bool DeviceAggregator::Refresh(word32 timestamp) {

	auto enter_debugger = this->_SelfRefresh(timestamp);
	
	for(auto iter = this->_AggregatedDevices->begin(); iter != this->_AggregatedDevices->end(); iter++)
		enter_debugger = enter_debugger || (*iter)->Refresh(timestamp);

	return enter_debugger;
}


bool DeviceAggregator::TryReadByte(word32 address, word32 timestamp, word32 emulFlags, byte &b) {

	auto read = this->Device::TryReadByte(address, timestamp, emulFlags, b);
	auto out_b = b;

	for(auto iter = this->_AggregatedDevices->begin(); iter != this->_AggregatedDevices->end(); iter++) {

		read = (*iter)->TryReadByte(address, timestamp, emulFlags, b) || read;
		out_b &= b; //Kinda-sorta simulate bus contention (failed reads default to 0xFF 'all pulled up', and devices responding on the bus pull zeroed lines to ground)
	}

	b = out_b;

	return read;
}

bool DeviceAggregator::TryWriteByte(word32 address, word32 timestamp, byte b) {
	
	auto written = this->Device::TryWriteByte(address, timestamp, b);

	for(auto iter = this->_AggregatedDevices->begin(); iter != this->_AggregatedDevices->end(); iter++)
		written = written || (*iter)->TryWriteByte(address, timestamp, b);

	return written;
}


