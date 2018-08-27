#include <Windows.h>
#include "ResponseRange.h"

ResponseRange::ResponseRange(word32 start, word32 end, byte rw_mask) : ResponseRange::ResponseRange(start, end, rw_mask, NULL, NULL) {}

ResponseRange::ResponseRange(word32 start, word32 end, byte rw_mask, ResponseRange::ReadCallback read_callback, ResponseRange::WriteCallback write_callback) {
	
	this->_Start = start & 0xFFFFFF;
	this->_End = end & 0xFFFFFF;
	this->_RWMask = rw_mask & RW_VALID_MASKS;
	this->_ReadCallback = read_callback;
	this->_WriteCallback = write_callback;
}

bool ResponseRange::RespondsToAddress(word32 address, bool write) {

	return (
		(
			((!write) && ((this->_RWMask & RW_MASK_R) != 0)) ||
			((write)  && ((this->_RWMask & RW_MASK_W) != 0))
		) && (
			this->_Start <= address &&
			this->_End >= address
		)
	);
}

word32 ResponseRange::Start() {
	return this->_Start;
}

word32 ResponseRange::End() {
	return this->_End;
}

ResponseRange::ReadCallback ResponseRange::GetReadCallback() {

	return this->_ReadCallback;
}

ResponseRange::WriteCallback ResponseRange::GetWriteCallback() {

	return this->_WriteCallback;
}
