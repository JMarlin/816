#pragma once

#include "device.h"

//Register address values from datasheet
#define XR_MRA     0x00 //RW
#define XR_SRA     0x01 //R
#define XR_CSRA    0x01 //W
#define XR_MISR    0x02 //R
#define XR_CRA     0x02 //W
#define XR_RHRA    0x03 //R
#define XR_THRA    0x03 //W
#define XR_IPCR    0x04 //R
#define XR_ACR     0x04 //W
#define XR_ISR     0x05 //R
#define XR_IMR     0x05 //W
#define XR_CTU     0x06 //RW
#define XR_CTL     0x07 //RW
#define XR_MRB     0x08 //RW
#define XR_SRB     0x09 //R
#define XR_CSRB    0x09 //W
#define XR_CRB     0x0A //W
#define XR_RHRB    0x0B //R
#define XR_THRB    0x0B //W
#define XR_IVR     0x0C //RW
#define XR_IP      0x0D //R
#define XR_OPCR    0x0D //W
#define XATC_SCC   0x0E //R
#define XATC_SOPBC 0x0E //W
#define XATC_STC   0x0F //R
#define XATC_COPBC 0x0F //W

//FIFO sizes
#define RHRA_DEPTH 3
#define RHRB_DEPTH 3

//ISR bit fields
#define ISR_INPUT_PORT_CHG_BIT   0b10000000
#define ISR_DATA_BRK_CHG_B_BIT   0b01000000
#define ISR_RXRDY_FFULL_B_BIT    0b00100000
#define ISR_TXRDYB_BIT           0b00010000
#define ISR_COUNTER_READY_BIT    0b00001000
#define ISR_DATA_BRK_CHG_A_BIT   0b00000100
#define ISR_RXRDY_FFULL_A_BIT    0b00000010
#define ISR_TXRDYA_BIT           0b00000001

//MR1n bit fields
#define MR1_RX_RTS_CONTROL_BIT 0b10000000
#define MR1_RX_INT_SELECT_BIT  0b01000000
#define MR1_ERROR_MODE_BIT     0b00100000
#define MR1_PARITY_MODE_BIS    0b00011000
#define MR1_PARITY_SELECT_BIT  0b00000100
#define MR1_BITS_PER_CHAR_BITS 0b00000011

#define RTS_CONTROL_YES   0b10000000
#define RTS_CONTROL_NO    0b00000000
#define RX_INT_SEL_FFULL  0b01000000
#define RX_INT_SEL_RXRDY  0b00000000
#define ERROR_MODE_BLOCK  0b00100000
#define ERROR_MODE_CHAR   0b00000000
#define PARITY_MODE_WITH  0b00000000
#define PARITY_MODE_FORCE 0b00001000
#define PARITY_MODE_NONE  0b00010000
#define PARITY_MODE_MULTI 0b00011000
#define PARITY_SEL_ODD    0b00000100
#define PARITY_SEL_EVEN   0b00000000
#define BITS_PER_CHAR_5   0b00000000
#define BITS_PER_CHAR_6   0b00000001
#define BITS_PER_CHAR_7   0b00000010
#define BITS_PER_CHAR_8   0b00000011

//MR2n bit fields
#define MR2_CHANNEL_MODE_BITS  0b11000000
#define MR2_TX_RTS_CONTROL_BIT 0b00100000
#define MR2_CTS_ENABLE_TX_BIT  0b00010000
#define MR2_STOP_BIT_LEN_BITS  0b00001111

#define CHANNEL_MODE_NORMAL 0b00000000
#define CHANNEL_MODE_ECHO   0b01000000
#define CHANNEL_MODE_LLOOP  0b10000000
#define CHANNEL_MODE_RLOOP  0b11000000
#define TX_RTS_CONTROL_YES  0b00100000
#define TX_RTS_CONTROL_NO   0b00000000
#define CTS_ENABLE_TX_YES   0b00010000
#define CTS_ENABLE_TX_NO    0b00000000
#define STOP_BIT_LEN_0_563  0b00000000
#define STOP_BIT_LEN_0_625  0b00000001
#define STOP_BIT_LEN_0_688  0b00000010
#define STOP_BIT_LEN_0_750  0b00000011
#define STOP_BIT_LEN_0_813  0b00000100
#define STOP_BIT_LEN_0_875  0b00000101
#define STOP_BIT_LEN_0_938  0b00000110
#define STOP_BIT_LEN_1_000  0b00000111
#define STOP_BIT_LEN_1_563  0b00001000
#define STOP_BIT_LEN_1_625  0b00001001
#define STOP_BIT_LEN_1_688  0b00001010
#define STOP_BIT_LEN_1_750  0b00001011
#define STOP_BIT_LEN_1_813  0b00001100
#define STOP_BIT_LEN_1_875  0b00001101
#define STOP_BIT_LEN_1_938  0b00001110
#define STOP_BIT_LEN_2_000  0b00001111

//SRA,SRB bit fields
#define SR_RECEIVED_BRK_BIT 0b10000000
#define SR_FRAMING_ERR_BIT  0b01000000
#define SR_PARITY_ERR_BIT   0b00100000
#define SR_OVERRUN_ERR_BIT  0b00010000
#define SR_TXEMT_BIT        0b00001000
#define SR_TXRDY_BIT        0b00000100
#define SR_FFULL_BIT        0b00000010
#define SR_RXRDY_BIT        0b00000001

class XR88C681 : public Device {

public:
	XR88C681(word32 base_addr);
	~XR88C681();

	static bool Read_MRA(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	static bool Read_SRA(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	static bool Read_MISR(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	static bool Read_RHRA(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	static bool Read_IPCR(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	static bool Read_ISR(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	static bool Read_CTU(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	static bool Read_CTL(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	static bool Read_MRB(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	static bool Read_SRB(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	static bool Read_RHRB(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	static bool Read_IVR(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	static bool Read_IP(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	static bool Read_SCC(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	static bool Read_STC(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);

	static bool Write_MRA(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	static bool Write_CSRA(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	static bool Write_CRA(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	static bool Write_THRA(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	static bool Write_ACR(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	static bool Write_IMR(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	static bool Write_CTU(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	static bool Write_CTL(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	static bool Write_MRB(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	static bool Write_CSRB(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	static bool Write_CRB(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	static bool Write_THRB(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	static bool Write_IVR(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	static bool Write_OPCR(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	static bool Write_SOPBC(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	static bool Write_COPBC(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);

	bool IntRead_MRA(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool IntRead_SRA(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool IntRead_MISR(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool IntRead_IPCR(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool IntRead_ISR(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool IntRead_RHRA(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool IntRead_CTU(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool IntRead_CTL(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool IntRead_MRB(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool IntRead_SRB(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool IntRead_RHRB(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool IntRead_IVR(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool IntRead_IP(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool IntRead_SCC(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool IntRead_STC(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool IntWrite_MRA(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	bool IntWrite_CSRA(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	bool IntWrite_ACR(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	bool IntWrite_IMR(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	bool IntWrite_CTU(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	bool IntWrite_CTL(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	bool IntWrite_MRB(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	bool IntWrite_CSRB(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	bool IntWrite_CRB(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	bool IntWrite_IVR(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	bool IntWrite_OPCR(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	bool IntWrite_SOPBC(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	bool IntWrite_COPBC(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	bool IntWrite_CRA(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	bool IntWrite_THRA(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);
	bool IntWrite_THRB(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);


	bool Refresh(word32 timestamp);

protected:
	bool _InternalReadByte(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b);
	bool _InternalWriteByte(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b);

private:
	void _RegisterRegister(word32 base, byte reg, byte rw_flag, ResponseRange::ReadCallback read_callback, ResponseRange::WriteCallback write_callback);

	void _SetISRWithSideEffects(byte value);

	bool _TXEnabledA;
	bool _RXEnabledA;
	bool _TXEnabledB;
	bool _RXEnabledB;
	bool _ZMode;
	bool _BreakingA;
	bool _BreakingB;
	bool _RXSelectExtendA;
	bool _TXSelectExtendA;
	bool _RXSelectExtendB;
	bool _TXSelectExtendB;
	bool _InStandby;
	byte* _MRPtrA;
	byte* _MRPtrB;
	byte _RHRA[RHRA_DEPTH];
	byte _RHRB[RHRB_DEPTH];
	byte _RHRAInPtr;
	byte _RHRAOutPtr;
	byte _RHRBInPtr;
	byte _RHRBOutPtr;

	byte _MRA1;
	byte _MRA2;
	byte _MRB1;
	byte _MRB2;
	byte _SRA;
	byte _CSRA;
	byte _CRA;
	byte _IPCR;
	byte _ACR;
	byte _ISR;
	byte _IMR;
	byte _CTU;
	byte _CTL;
	byte _MRB;
	byte _SRB;
	byte _CSRB;
	byte _CRB;
	byte _THRA;
	byte _THRB;
	byte _IVR;
	byte _OPCR;
};