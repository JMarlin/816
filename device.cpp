#include "device.h"

Device::Device() {

	this->_InitOk = false;
	this->_ResponseRanges = new std::list<ResponseRange*>();
}

Device::~Device() {

	for(auto iter = this->_ResponseRanges->begin(); iter != this->_ResponseRanges->end(); iter++)
		delete *iter;

	delete this->_ResponseRanges;
}

bool Device::GetInitOk() {
	return this->_InitOk;
}

bool Device::TryReadByte(word32 address, word32 timestamp, word32 emulFlags, byte &b) {

	auto range = this->_MatchRange(address, false);

	if(range == NULL) {

		b = 0xFF;
		return false;
	}

	auto callback = range->GetReadCallback();

	if(callback == NULL)
		return this->_InternalReadByte(address, timestamp, emulFlags, range, b);

	return callback(this, address, timestamp, emulFlags, range, b);
}

bool Device::TryWriteByte(word32 address, word32 timestamp, byte b) {

	auto range = this->_MatchRange(address, true);

	if(range == NULL)
		return false;

	auto callback = range->GetWriteCallback();

	if(callback == NULL)
		return this->_InternalWriteByte(address, timestamp, range, b);

	return callback(this, address, timestamp, range, b);
}

ResponseRange* Device::AddResponseRange(word32 start, word32 end, byte rw_mask) {

	return this->AddResponseRange(start, end, rw_mask, NULL, NULL);
}

ResponseRange* Device::AddResponseRange(word32 start, word32 end, byte rw_mask, ResponseRange::ReadCallback read_callback, ResponseRange::WriteCallback write_callback) {

	auto range = new ResponseRange(start, end, rw_mask, read_callback, write_callback);

	this->_ResponseRanges->push_back(range);

	return range;
}

ResponseRange* Device::_MatchRange(word32 address, bool write) {

	for(auto iter = this->_ResponseRanges->begin(); iter != this->_ResponseRanges->end(); iter++)
		if((*iter)->RespondsToAddress(address, write))
			return *iter;

	return NULL;
}