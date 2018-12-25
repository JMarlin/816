#include "XR88C681.h"

XR88C681::XR88C681(word32 base_addr) {
	
	base_addr &= 0x00FFFFFC;

	//File transfer state variable init
	this->_SendingFile = false;

	//Init register statuses
	this->_TXEnabledA = false;
	this->_RXEnabledA = false;
	this->_TXEnabledB = false;
	this->_RXEnabledB = false;
	this->_ZMode = false;
	this->_BreakingA = false;
	this->_BreakingB = false;
	this->_RXSelectExtendA = false; //TODO: CHECK WHAT THIS IS SUPPOSED TO BE AT STARTUP
	this->_TXSelectExtendA = false; //TODO: ''
	this->_RXSelectExtendB = false; //TODO: ''
	this->_TXSelectExtendB = false; //TODO: ''
	this->_InStandby = false;
	this->_MRPtrA = &this->_MRA1;
	this->_MRPtrB = &this->_MRB1;
	this->_RHRAInPtr = 0;
	this->_RHRBInPtr = 0;
	this->_RHRAOutPtr = 0;
	this->_RHRBOutPtr = 0;

	//Attach register r/w handlers
	this->_RegisterRegister(base_addr, XR_MRA,     RW_MASK_R | RW_MASK_W,  XR88C681::Read_MRA,  XR88C681::Write_MRA);
	this->_RegisterRegister(base_addr, XR_SRA,     RW_MASK_R,              XR88C681::Read_SRA,  NULL);
	this->_RegisterRegister(base_addr, XR_CSRA,    RW_MASK_W,              NULL,                XR88C681::Write_CSRA);
	this->_RegisterRegister(base_addr, XR_MISR,    RW_MASK_R,              XR88C681::Read_MISR, NULL);
	this->_RegisterRegister(base_addr, XR_CRA,     RW_MASK_W,              NULL,                XR88C681::Write_CRA);
	this->_RegisterRegister(base_addr, XR_RHRA,    RW_MASK_R,              XR88C681::Read_RHRA, NULL);
	this->_RegisterRegister(base_addr, XR_THRA,    RW_MASK_W,              NULL,                XR88C681::Write_THRA);
	this->_RegisterRegister(base_addr, XR_IPCR,    RW_MASK_R,              XR88C681::Read_IPCR, NULL);
	this->_RegisterRegister(base_addr, XR_ACR,     RW_MASK_W,              NULL,                XR88C681::Write_ACR);
	this->_RegisterRegister(base_addr, XR_ISR,     RW_MASK_R,              XR88C681::Read_ISR,  NULL);
	this->_RegisterRegister(base_addr, XR_IMR,     RW_MASK_W,              NULL,                XR88C681::Write_IMR);
	this->_RegisterRegister(base_addr, XR_CTU,     RW_MASK_R | RW_MASK_W,  XR88C681::Read_CTU,  XR88C681::Write_CTU);
	this->_RegisterRegister(base_addr, XR_CTL,     RW_MASK_R | RW_MASK_W,  XR88C681::Read_CTL,  XR88C681::Write_CTL);
	this->_RegisterRegister(base_addr, XR_MRB,     RW_MASK_R | RW_MASK_W,  XR88C681::Read_MRB,  XR88C681::Write_MRB);
	this->_RegisterRegister(base_addr, XR_SRB,     RW_MASK_R,              XR88C681::Read_SRB,  NULL);
	this->_RegisterRegister(base_addr, XR_CSRB,    RW_MASK_W,              NULL,                XR88C681::Write_CSRB);
	this->_RegisterRegister(base_addr, XR_CRB,     RW_MASK_W,              NULL,                XR88C681::Write_CRB);
	this->_RegisterRegister(base_addr, XR_RHRB,    RW_MASK_R,              XR88C681::Read_RHRB, NULL);
	this->_RegisterRegister(base_addr, XR_THRB,    RW_MASK_W,              NULL,                XR88C681::Write_THRB);
	this->_RegisterRegister(base_addr, XR_IVR,     RW_MASK_R | RW_MASK_W,  XR88C681::Read_IVR,  XR88C681::Write_IVR);
	this->_RegisterRegister(base_addr, XR_IP,      RW_MASK_R,              XR88C681::Read_IP,   NULL);
	this->_RegisterRegister(base_addr, XR_OPCR,    RW_MASK_W,              NULL,                XR88C681::Write_OPCR);
	this->_RegisterRegister(base_addr, XATC_SCC,   RW_MASK_R,              XR88C681::Read_SCC,  NULL);
	this->_RegisterRegister(base_addr, XATC_SOPBC, RW_MASK_W,              NULL,                XR88C681::Write_SOPBC);
	this->_RegisterRegister(base_addr, XATC_STC,   RW_MASK_R,              XR88C681::Read_STC,  NULL);
	this->_RegisterRegister(base_addr, XATC_COPBC, RW_MASK_W,              NULL,                XR88C681::Write_COPBC);

	//Set up and wait for connection to console on TCP
	printf("Waiting on console to attach to TCP 50615...");
	
	if (this->_GetTCPConsole()) {

		//Hack to turn off local echo on the client
		this->_TCPConsolePutch(0xFF); //IAC
		this->_TCPConsolePutch(0xFB); //WILL
		this->_TCPConsolePutch(0x01); //ECHO

		this->_InitOk = true;
		printf("done\n");
	} else {

		printf("unable to establish connection.\n");
	}
}

#ifdef _WIN32

bool XR88C681::_GetTCPConsole() {

	WSADATA wsa_data;
	struct addrinfo * addr_info = NULL;
	struct addrinfo hints;

	SOCKET listen_socket = INVALID_SOCKET;
	this->_ClientSocket = INVALID_SOCKET;

	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
		return false;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, "50615", &hints, &addr_info) != 0) {

		WSACleanup();
		return false;
	}

	if (
		(listen_socket = socket(
			addr_info->ai_family,
			addr_info->ai_socktype,
			addr_info->ai_protocol)
		) == INVALID_SOCKET
	) {
		
		freeaddrinfo(addr_info);
		WSACleanup();
		return false;
	}

	if (bind(
			listen_socket, 
			addr_info->ai_addr, 
			(int)addr_info->ai_addrlen
	) == SOCKET_ERROR) {
		
		freeaddrinfo(addr_info);
		closesocket(listen_socket);
		WSACleanup();
		return false;
	}

	freeaddrinfo(addr_info);

	if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
	
		closesocket(listen_socket);
		WSACleanup();
		return false;
	}

	if ((this->_ClientSocket = accept(listen_socket, NULL, NULL))
		== INVALID_SOCKET) {

		closesocket(listen_socket);
		WSACleanup();
		return false;
	}

	closesocket(listen_socket);

	return true;
}

void XR88C681::_CloseTCPConsole() {

	if (this->_ClientSocket == INVALID_SOCKET)
		return;

	closesocket(this->_ClientSocket);
	WSACleanup();
}

bool XR88C681::_TCPConsolePending() {
	
	u_long bytes_available = 0;

	ioctlsocket(this->_ClientSocket, FIONREAD, &bytes_available);

	return bytes_available > 0;
}

#else

bool XR88C681::_GetTCPConsole() {

	int listen_fd = 0;

	struct sockaddr_in server_address;

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&server_address, '0', sizeof(server_address));
	
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(50615);

	bind(listen_fd, (struct sockaddr*)&server_address, sizeof(server_address));

	listen(listen_fd, 10);

	this->_ClientSocket = accept(listen_fd, (struct sockaddr*)0, 0);
	
	close(listen_fd);

	return true;
}

void XR88C681::_CloseTCPConsole() {

	if (this->_ClientSocket == 0)
		return;

	close(this->_ClientSocket);
}

bool XR88C681::_TCPConsolePending() {
	
	int count;

	ioctl(this->_ClientSocket, FIONREAD, &count);

	return count > 0;
}

#endif 


byte XR88C681::_TCPConsoleGetch() {
	
	char buf;

	recv(this->_ClientSocket, &buf, 1, 0);

	return buf;
}

void XR88C681::_TCPConsolePutch(byte b) {

	const char val = b;

	send(this->_ClientSocket, &val, 1, 0);
}

XR88C681::~XR88C681() {
	this->_CloseTCPConsole();
}

bool XR88C681::StartSendFile(char* filename) {

	FILE* send_file = fopen(filename, "rb");

	if (!send_file) {

		printf("Couldn't find file '%s'.\n", filename);
		return false;
	}

	fseek(send_file, 0, SEEK_END);
	this->_SendFileSize = ftell(send_file);
	rewind(send_file);

	this->_SendFileData = (unsigned char*)malloc(this->_SendFileSize);

	if (!this->_SendFileData) {

		printf("Couldn't allocate memory for file.\n");
		fclose(send_file);
		return false;
	}

	int read_size;

	if ((read_size = fread(this->_SendFileData, 1, this->_SendFileSize, send_file)) != this->_SendFileSize) {

		printf("Couldn't read file, %i != %i.\n", this->_SendFileSize, read_size);
		fclose(send_file);
		free(this->_SendFileData);
		this->_SendFileData = 0;
		return false;
	}

	fclose(send_file);
	this->_SendFileIndex = 0;
	this->_SendingFile = true;

	printf("File %s loaded and queued.\n", filename);

	return true;
}

//This should be used in any case that we want to set an ISR bit internally as it will
//automatically check the new ISR value against the IMR and trigger an interrupt if required
void XR88C681::_SetISRWithSideEffects(byte value) {

	this->_ISR = value;

	//TODO: Schedule interrupt on 816 if ISR & IMR != 0
}

void XR88C681::_RegisterRegister(word32 base, byte reg, byte rw_flag, ResponseRange::ReadCallback read_callback, ResponseRange::WriteCallback write_callback) {

	this->AddResponseRange(
		base + reg,
		base + reg,
		rw_flag,
		read_callback,
		write_callback
	);
}

bool XR88C681::Refresh(word32 timestamp) {

	//We're set up to assume that the console is a TTY connected to channel A
	//(so we don't really do anything for channel B

	//TODO: We really should dump THRB bytes for proper emulation, even though they're
	//      not going anywhere

	//We emulate an incoming console character as a new character having come down 
	//RX on channel A

	bool kb_was_hit = false;

	if(
		(this->_SendingFile && ((this->_SRA & SR_FFULL_BIT) == 0)) || 
	    (kb_was_hit = this->_TCPConsolePending())
	) {

		auto keyval = kb_was_hit ? this->_TCPConsoleGetch() : 0;

		//Trip the debugger on '`'
		if (keyval == '`')
			return true;

		auto received = 
			this->_SendingFile ? 
				this->_SendFileIndex == this->_SendFileSize ?
					this->_SendFileIndex++, 0 :
					this->_SendFileData[this->_SendFileIndex++] :
				keyval;

		//If the FIFO is already full, we need to dump the incoming char and
		//flag an overrun condition
		if((this->_SRA & SR_FFULL_BIT) != 0) {

			this->_SRA |= SR_OVERRUN_ERR_BIT;
		} else if (this->_RXEnabledA && (!this->_InStandby)) {

		//Getting bytes on the line does nothing if the channel A receiver is 
		//disabled or if the whole chip is in standby

			//Check line configuration -- if it's not 8n1, 9600 then we produce
			//pseudorandom garbage to simulate mismatched connection params
			if(((this->_MRA1 & 0b00011111) != 0b00010011) || //No parity, even parity, 8 bits per char
			   ((this->_MRA2 & 0b11011111) != 0b00000111) || //Normal channel mode, don't disable TX via CTS, stop bit length = 1.000
			   ((this->_CSRA & 0b11110000) != 0b10110000))   //Baud rate of receiver is based on CSR[7:4], ACR[7], and the select extend bit, but 9600 baud is always acheived with CSR[7:4] = 0b1011 regardless of the settings of the latter two items
				received = rand() & 0xFF; //Receive a bad byte if the above conditions aren't met

			//Insert incoming character into the FIFO
			this->_RHRA[this->_RHRAInPtr] = received;

			//Wrap advance the FIFO pointer and check for FIFO full case
			this->_RHRAInPtr = (this->_RHRAInPtr + 1) % RHRA_DEPTH;
			
			//If the FIFO filled up, update the SR and possibly trigger the interrupt
			if(this->_RHRAInPtr == this->_RHRAOutPtr) {

				this->_SRA |= SR_FFULL_BIT;

				if((this->_MRA1 & MR1_RX_INT_SELECT_BIT) == RX_INT_SEL_FFULL)
					this->_SetISRWithSideEffects(this->_ISR | ISR_RXRDY_FFULL_A_BIT);
			}

			//Finally, since we successfully received a character, indicate this in the
			//status register and potentially fire an interrupt
			this->_SRA |= SR_RXRDY_BIT;

			if((this->_MRA1 & MR1_RX_INT_SELECT_BIT) == RX_INT_SEL_RXRDY)
				this->_SetISRWithSideEffects(this->_ISR | ISR_RXRDY_FFULL_A_BIT);
		}

		if(this->_SendingFile && (this->_SendFileIndex > this->_SendFileSize)) {
			
			this->_SendingFile = false;
			free(this->_SendFileData);
		}
	}

	//When we're ready to send a character out TX A (which we are not currently
	//emulating, but certainly could be by checking the system time to delay 
	//the next send until 10/9600ths of a second have elapsed), we first check
	//to see whether the transmitter is able to send (we must not be in standby,
	//nor have a disabled transmitter, and ISR must indicate that a byte is ready
	//to send)
	if(this->_TXEnabledA &&
	   (!this->_InStandby) &&
	   (!(this->_SRA & SR_TXRDY_BIT)) &&
	   (!(this->_SRA & SR_TXEMT_BIT))) {
	
		//Check line configuration -- if it's not 8n1, 9600 then we produce
		//pseudorandom garbage to simulate mismatched connection params
		if(((this->_MRA1 & 0b00011111) != 0b00010011) || //No parity, even parity, 8 bits per char
		   ((this->_MRA2 & 0b11011111) != 0b00000111) || //Normal channel mode, don't disable TX via CTS, stop bit length = 1.000
		   ((this->_CSRA & 0b00001111) != 0b00001011))   //Baud rate of transmitter is based on CSR[3:0], ACR[7], and the select extend bit, but 9600 baud is always acheived with CSR[3:0] = 0b1011 regardless of the settings of the latter two items
			this->_THRA = rand() & 0xFF; //Send a bad byte if the above conditions aren't met

		//Print the character transmitted by the DUART
		this->_TCPConsolePutch(this->_THRA);
		fflush(stdout);

		//Update the status register (THRB is empty and ready to transmit)
		this->_SRA |= SR_TXRDY_BIT | SR_TXEMT_BIT;

		//Indicate that the device is ready to send again
		this->_SetISRWithSideEffects(this->_ISR | ISR_TXRDYA_BIT);
	}
	  
	//Same thing for port B for completeness, but we dump the bytes off into space
	if(this->_TXEnabledB &&
	   (!this->_InStandby) &&
	   (!(this->_SRB & SR_TXRDY_BIT)) &&
	   (!(this->_SRB & SR_TXEMT_BIT))) {

		//Update the status register (THRB is empty and ready to transmit)
		this->_SRB |= SR_TXRDY_BIT | SR_TXEMT_BIT;

		//Update the interrupt register
		this->_SetISRWithSideEffects(this->_ISR | ISR_TXRDYB_BIT);
	}

	return false;
}

bool XR88C681::_InternalReadByte(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* range, byte &b) {
	
	return false; //All reads and writes should go through our response range access callbacks
}

bool XR88C681::_InternalWriteByte(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	
	return false; //All reads and writes should go through our response range access callbacks
}



//------------------------------------------------------------------
//    Read callbacks
//------------------------------------------------------------------


bool XR88C681::Read_MRA(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) { 
	return ((XR88C681*)this_ptr)->IntRead_MRA(address, timestamp, emulFlags, triggered_range, b);
}

bool XR88C681::IntRead_MRA(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	
	b = *(this->_MRPtrA);

	//After any read or write to MRA, the MRA pointer automatically gets set to MR2A and only an
	//MRA reset command will switch the MRA pointer back to MR1A
	this->_MRPtrA = &this->_MRA2;

	return true;
}

bool XR88C681::Read_SRA(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	return ((XR88C681*)this_ptr)->IntRead_SRA(address, timestamp, emulFlags, triggered_range, b);
}

bool XR88C681::IntRead_SRA(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	
	b = this->_SRA;

	return true;
}

bool XR88C681::Read_MISR(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	return ((XR88C681*)this_ptr)->IntRead_MISR(address, timestamp, emulFlags, triggered_range, b);
}

bool XR88C681::IntRead_MISR(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {

	//MISR is literally just IMR and ISR ANDed together
	b = this->_ISR & this->_IMR;

	return true;
}

bool XR88C681::Read_RHRA(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	return ((XR88C681*)this_ptr)->IntRead_RHRA(address, timestamp, emulFlags, triggered_range, b);
}

bool XR88C681::IntRead_RHRA(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {

	//Return whatever the FIFO happens to be pointing at (not sure if this is actual condition)
	b = this->_RHRA[this->_RHRAOutPtr];

	//Only advance if the FIFO wasn't empty
	//The FIFO is empty if the pointers are the same and the FIFO isn't full
	if((this->_RHRAInPtr != this->_RHRAOutPtr) ||
	   (this->_SRA & SR_FFULL_BIT)) {

		//Advance with wrap
		this->_RHRAOutPtr = (this->_RHRAOutPtr + 1) % RHRA_DEPTH;

		//If we consumed a character, it's not possible that the FIFO is full now
		this->_SRA &= ~SR_FFULL_BIT;

		//Lower the RX full/ready ISR value if we were in RX full interrupt mode
		if((this->_MRA1 & MR1_RX_INT_SELECT_BIT) == RX_INT_SEL_FFULL)
			this->_SetISRWithSideEffects(this->_ISR & ~ISR_RXRDY_FFULL_A_BIT);

		//If the advanced pointer ran into the input pointer, that means the read
		//emptied the FIFO and we must set status accordingly
		if(this->_RHRAInPtr == this->_RHRAOutPtr) {

			this->_SRA &= ~SR_RXRDY_BIT;

			//Lower the RX full/ready ISR value if we were in RX ready interrupt mode
			if ((this->_MRA1 & MR1_RX_INT_SELECT_BIT) == RX_INT_SEL_RXRDY)
				this->_SetISRWithSideEffects(this->_ISR & ~ISR_RXRDY_FFULL_A_BIT);
		}
	}	

	return true;
}

bool XR88C681::Read_IPCR(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	return ((XR88C681*)this_ptr)->IntRead_IPCR(address, timestamp, emulFlags, triggered_range, b);
}

bool XR88C681::IntRead_IPCR(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	
	//Reading IPCR resets input port changed interrupt
	this->_SetISRWithSideEffects(this->_ISR & ~ISR_INPUT_PORT_CHG_BIT);

	return true;
}

bool XR88C681::Read_ISR(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	return ((XR88C681*)this_ptr)->IntRead_ISR(address, timestamp, emulFlags, triggered_range, b);
}

bool XR88C681::IntRead_ISR(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {

	b = this->_ISR;

	return true;
}

//TODO: None of the counter system is currently implemented. At this point, CTU and CTL
//      are effectively just some general purpose registers
bool XR88C681::Read_CTU(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	return ((XR88C681*)this_ptr)->IntRead_CTU(address, timestamp, emulFlags, triggered_range, b);
}

bool XR88C681::IntRead_CTU(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {

	b = this->_CTU;

	return true;
}

bool XR88C681::Read_CTL(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	return ((XR88C681*)this_ptr)->IntRead_CTL(address, timestamp, emulFlags, triggered_range, b);
}

bool XR88C681::IntRead_CTL(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {

	b = this->_CTL;

	return true;
}

bool XR88C681::Read_MRB(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	return ((XR88C681*)this_ptr)->IntRead_MRB(address, timestamp, emulFlags, triggered_range, b);
}

bool XR88C681::IntRead_MRB(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {

	b = *(this->_MRPtrB);

	//After any read or write to MRB, the MRB pointer automatically gets set to MR2B and only an
	//MRA reset command will switch the MRB pointer back to MR1B
	this->_MRPtrB = &this->_MRB2;

	return true;
}

bool XR88C681::Read_SRB(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	return ((XR88C681*)this_ptr)->IntRead_SRB(address, timestamp, emulFlags, triggered_range, b);
}

bool XR88C681::IntRead_SRB(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {

	b = this->_SRB;

	return true;
}

bool XR88C681::Read_RHRB(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	return ((XR88C681*)this_ptr)->IntRead_RHRB(address, timestamp, emulFlags, triggered_range, b);
}

bool XR88C681::IntRead_RHRB(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {

	//Return whatever the FIFO happens to be pointing at (not sure if this is actual condition)
	b = this->_RHRB[this->_RHRBOutPtr];

	//Only advance if the FIFO wasn't empty
	//The FIFO is empty if the pointers are the same and the FIFO isn't full
	if ((this->_RHRBInPtr != this->_RHRBOutPtr) ||
		(this->_SRB & SR_FFULL_BIT)) {

		//Advance with wrap
		this->_RHRBOutPtr = (this->_RHRBOutPtr + 1) % RHRB_DEPTH;

		//If we consumed a character, it's not possible that the FIFO is full now
		this->_SRB &= ~SR_FFULL_BIT;

		//Lower the RX full/ready ISR value if we were in RX full interrupt mode
		if ((this->_MRB1 & MR1_RX_INT_SELECT_BIT) == RX_INT_SEL_FFULL)
			this->_SetISRWithSideEffects(this->_ISR & ~ISR_RXRDY_FFULL_B_BIT);

		//If the advanced pointer ran into the input pointer, that means the read
		//emptied the FIFO and we must set status accordingly
		if (this->_RHRBInPtr == this->_RHRBOutPtr) {

			this->_SRB &= ~SR_RXRDY_BIT;

			//Lower the RX full/ready ISR value if we were in RX ready interrupt mode
			if ((this->_MRB1 & MR1_RX_INT_SELECT_BIT) == RX_INT_SEL_RXRDY)
				this->_SetISRWithSideEffects(this->_ISR & ~ISR_RXRDY_FFULL_B_BIT);
		}
	}

	return true;
}

//At this moment, Z-mode isn't really emulated and so the IVR remains a general purpose register
bool XR88C681::Read_IVR(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	return ((XR88C681*)this_ptr)->IntRead_IVR(address, timestamp, emulFlags, triggered_range, b);
}

bool XR88C681::IntRead_IVR(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {

	b = this->_IVR;

	return true;
}

//In our emulation, IP is currently emulated to have none of the pins connected
bool XR88C681::Read_IP(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	return ((XR88C681*)this_ptr)->IntRead_IP(address, timestamp, emulFlags, triggered_range, b);
}

bool XR88C681::IntRead_IP(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {

	b = 0xFF; //Complete guess here that the input port pins float high

	return true;
}

//Counter/timer not at all implemented in the current emulation
//Otherwise, this method should return nothing and start the counter/timer going
bool XR88C681::Read_SCC(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	return ((XR88C681*)this_ptr)->IntRead_SCC(address, timestamp, emulFlags, triggered_range, b);
}

bool XR88C681::IntRead_SCC(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {

	b = 0x00;

	return true;
}

//Counter/timer not at all implemented in the current emulation
//Otherwise, this method should return nothing and halt the counter/timer going
bool XR88C681::Read_STC(Device* this_ptr, word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {
	return ((XR88C681*)this_ptr)->IntRead_STC(address, timestamp, emulFlags, triggered_range, b);
}

bool XR88C681::IntRead_STC(word32 address, word32 timestamp, word32 emulFlags, ResponseRange* triggered_range, byte& b) {

	b = 0x00;

	return true;
}



//------------------------------------------------------------------
//    Write callbacks
//------------------------------------------------------------------


bool XR88C681::Write_MRA(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	return ((XR88C681*)this_ptr)->IntWrite_MRA(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_MRA(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {

	*(this->_MRPtrA) = b;

	//After any read or write to MRA, the MRA pointer automatically gets set to MR2A and only an
	//MRA reset command will switch the MRA pointer back to MR1A
	this->_MRPtrA = &this->_MRA2;

	return true;
}

//Clocking and baud rate are fundamentally ignored at the moment, so we just stash the
//value of CSRA and then do nothing with it
bool XR88C681::Write_CSRA(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	return ((XR88C681*)this_ptr)->IntWrite_CSRA(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_CSRA(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {

	this->_CSRA = b;

	return true;
}

#define CR_TX_ENABLE_PORTION(x) ((x) & 0x03)
#define CR_RX_ENABLE_PORTION(x) (((x) >> 2) & 0x03)
#define CR_COMMAND_PORTION(x) (((x) >> 4) & 0x0F)
#define CR_RXTX_NOCHANGE_CMD 0b00
#define CR_RXTX_ENABLE_CMD   0b01
#define CR_RXTX_DISABLE_CMD  0x10
#define CR_RXTX_INVALID_CMD  0x11
#define CR_CMD_NULL              0b0000  //Does nothing
#define CR_CMD_RESET_MRPTR       0b0001  //Sets the channel's mode register pointer back to MRn1
#define CR_CMD_RESET_RX          0b0010  //Same as hardware reset: RX is disabled and RX FIFO is flushed
#define CR_CMD_RESET_TX          0b0011  //Same as hardware reset: TX is disabled and TX FIFO is flushed
#define CR_CMD_RESET_ERR         0b0100  //Clears SR[7:3] (RB, PE, FE, and OE)
#define CR_CMD_RESET_BRK_CHG_INT 0b0101  //Clears the channel's break change interrupt status bit
#define CR_CMD_START_BREAK       0b0110  //Only has an effect if TX is enabled, forces TX low
#define CR_CMD_STOP_BREAK        0b0111  //Return TX to high state if TX was breaking
#define CR_CMD_SET_RX_SEL_EX_BIT 0b1000  //Sets the channel's "Receiver BRG Select Extend Bit" to 1
#define CR_CMD_CLR_RX_SEL_EX_BIT 0b1001  //Clears ''
#define CR_CMD_SET_TX_SEL_EX_BIT 0b1010  //Sets the channel's "Transmitter BRG Selet Extend Bit" to 1
#define CR_CMD_CLR_TX_SEL_EX_BIT 0b1011  //Clears ''
#define CR_CMD_SET_STANDBY_VIA_A 0b1100  //When sent to channel A, power is removed from the internal modules of the device to put it into standby mode
#define CR_CMD_RESET_IUS_VIA_B   0b1100  //When sent to channel B (and the UART is in Z-mode), this resets the Interrupt-Under-Service latch and therefore sets IEO high
#define CR_CMD_SET_ACTIVE_VIA_A  0b1101  //When sent to channel A, takes the chip out of standby mode (if it was in it)
#define CR_CMD_SET_Z_MODE_VIA_B  0b1101  //When sent to channel B, puts the chip into Z-mode (Z80-type interrupt compatibility mode)
#define CR_CMD_RESERVED_1        0b1110  //RESERVED
#define CR_CMD_RESERVED_2        0b1111  //RESERVED

bool XR88C681::Write_CRA(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) { 
	return ((XR88C681*)this_ptr)->IntWrite_CRA(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_CRA(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	
	auto tx_enable_cmd = CR_TX_ENABLE_PORTION(b);
	auto rx_enable_cmd = CR_RX_ENABLE_PORTION(b);
	auto cr_command    = CR_COMMAND_PORTION(b);

	this->_TXEnabledA =
		tx_enable_cmd == CR_RXTX_DISABLE_CMD ?
			false :
			tx_enable_cmd == CR_RXTX_ENABLE_CMD ?
				true :
				this->_TXEnabledA;

	this->_RXEnabledA =
		rx_enable_cmd == CR_RXTX_DISABLE_CMD ?
			false :
			rx_enable_cmd == CR_RXTX_ENABLE_CMD ?
				true :
				this->_RXEnabledA;

	switch(cr_command) {

		case CR_CMD_RESET_MRPTR:
			this->_MRPtrA = &this->_MRA1;
			break;

		case CR_CMD_RESET_RX:
			this->_RXEnabledA = false;
			this->_SRA &= ~SR_FFULL_BIT; //TODO: Need to handle interrupt status here
			this->_SRA &= ~SR_RXRDY_BIT;
			this->_RHRAInPtr = 0;
			this->_RHRAOutPtr = 0;
			break;

		case CR_CMD_RESET_TX:
			this->_TXEnabledA = false;
			this->_SRA |= SR_TXRDY_BIT; //TODO: Need to handle interrupt status here
			this->_SRA |= SR_TXEMT_BIT;
			break;

		case CR_CMD_RESET_ERR:
			this->_SRA &= 0b00000111;
			break;

		case CR_CMD_RESET_BRK_CHG_INT:
			this->_ISR &= ~ISR_DATA_BRK_CHG_A_BIT;
			break;

		case CR_CMD_START_BREAK:
			this->_BreakingA = this->_TXEnabledA && true;
			break;

		case CR_CMD_STOP_BREAK:
			this->_BreakingA = false;
			break;

		case CR_CMD_SET_RX_SEL_EX_BIT:
			this->_RXSelectExtendA = true;
			break;

		case CR_CMD_CLR_RX_SEL_EX_BIT:
			this->_RXSelectExtendA = false;
			break;

		case CR_CMD_SET_TX_SEL_EX_BIT:
			this->_TXSelectExtendA = true;
			break;

		case CR_CMD_CLR_TX_SEL_EX_BIT:
			this->_TXSelectExtendA = false;
			break;

		case CR_CMD_SET_STANDBY_VIA_A:
			this->_InStandby = true;
			break;

		case CR_CMD_SET_ACTIVE_VIA_A:
			this->_InStandby = false;
			break;

		default:
			//Could trigger a log of 'unknown/invalid command'
			break;
	}

	return true;
}

bool XR88C681::Write_THRA(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	return ((XR88C681*)this_ptr)->IntWrite_THRA(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_THRA(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {

	//If TX is disabled, the device is in standby, 
	//or the status bit indicating data is in THRA is set, 
	//return early discarding b
	if((!this->_TXEnabledA)          ||
	   this->_InStandby             ||
	   !(this->_SRA & SR_TXRDY_BIT) ||
	   !(this->_SRA & SR_TXEMT_BIT)) 
		return false;

	//Set THRA value to be transmitted
	this->_THRA = b;

	//Update status to indicate that THRA is no longer empty
	this->_SRA &= ~(SR_TXEMT_BIT | SR_TXRDY_BIT);
	this->_SetISRWithSideEffects(this->_ISR & ~ISR_TXRDYA_BIT);

	return false;
}

//ACR handles input port interrupt configuration and counter #1 configuration
//We ignore both the input port and counters, so this gets written but never used
bool XR88C681::Write_ACR(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	return ((XR88C681*)this_ptr)->IntWrite_ACR(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_ACR(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {

	this->_ACR = b;

	return true;
}

bool XR88C681::Write_IMR(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	return ((XR88C681*)this_ptr)->IntWrite_IMR(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_IMR(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {

	this->_IMR = b;

	return true;
}

//We aren't emulating the counters, so CTU and CTL are effectively currently general purpose regs
bool XR88C681::Write_CTU(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	return ((XR88C681*)this_ptr)->IntWrite_CTU(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_CTU(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {

	this->_CTU = b;

	return true;
}

//We aren't emulating the counters, so CTU and CTL are effectively currently general purpose regs
bool XR88C681::Write_CTL(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	return ((XR88C681*)this_ptr)->IntWrite_CTL(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_CTL(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {

	this->_CTL = b;

	return true;
}

bool XR88C681::Write_MRB(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	return ((XR88C681*)this_ptr)->IntWrite_MRB(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_MRB(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {

	*(this->_MRPtrB) = b;

	//Bfter any read or write to MRB, the MRB pointer automatically gets set to MR2B and only an
	//MRB reset command will switch the MRB pointer back to MR1B
	this->_MRPtrB = &this->_MRB2;

	return true;
}

//Clocking and baud rate are fundamentally ignored at the moment, so we just stash the
//value of CSRB and then do nothing with it
bool XR88C681::Write_CSRB(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	return ((XR88C681*)this_ptr)->IntWrite_CSRB(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_CSRB(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {

	this->_CSRB = b;

	return true;
}

bool XR88C681::Write_CRB(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	return ((XR88C681*)this_ptr)->IntWrite_CRB(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_CRB(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {

	auto tx_enable_cmd = CR_TX_ENABLE_PORTION(b);
	auto rx_enable_cmd = CR_RX_ENABLE_PORTION(b);
	auto cr_command = CR_COMMAND_PORTION(b);

	this->_TXEnabledB =
		tx_enable_cmd == CR_RXTX_DISABLE_CMD ?
		false :
		tx_enable_cmd == CR_RXTX_ENABLE_CMD ?
		true :
		this->_TXEnabledB;

	this->_RXEnabledB =
		rx_enable_cmd == CR_RXTX_DISABLE_CMD ?
		false :
		rx_enable_cmd == CR_RXTX_ENABLE_CMD ?
		true :
		this->_RXEnabledB;

	switch (cr_command) {

		case CR_CMD_RESET_MRPTR:
			this->_MRPtrB = &this->_MRB1;
			break;

		case CR_CMD_RESET_RX:
			this->_RXEnabledB = false;
			break;

		case CR_CMD_RESET_TX:
			this->_TXEnabledB = false;
			break;

		case CR_CMD_RESET_ERR:
			this->_SRB &= 0b00000111;
			break;

		case CR_CMD_RESET_BRK_CHG_INT:
			this->_ISR &= ~ISR_DATA_BRK_CHG_B_BIT;
			break;

		case CR_CMD_START_BREAK:
			this->_BreakingB = this->_TXEnabledB && true;
			break;

		case CR_CMD_STOP_BREAK:
			this->_BreakingB = false;
			break;

		case CR_CMD_SET_RX_SEL_EX_BIT:
			this->_RXSelectExtendB = true;
			break;

		case CR_CMD_CLR_RX_SEL_EX_BIT:
			this->_RXSelectExtendB = false;
			break;

		case CR_CMD_SET_TX_SEL_EX_BIT:
			this->_TXSelectExtendB = true;
			break;

		case CR_CMD_CLR_TX_SEL_EX_BIT:
			this->_TXSelectExtendB = false;
			break;

		case CR_CMD_RESET_IUS_VIA_B:
			//This affects the IUS latch which in turn affects the IEO pin
			//We don't emulate either of these things
			break;

		case CR_CMD_SET_Z_MODE_VIA_B:
			this->_ZMode = true;
			break;

		default:
			//Could trigger a log of 'unknown/invalid command'
			break;
	}

	return true;
}

bool XR88C681::Write_THRB(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	return ((XR88C681*)this_ptr)->IntWrite_THRB(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_THRB(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {

	//If TX is disabled, the device is in standby, 
	//or the status bit indicating data is in THRA is set, 
	//return early discarding b
	if ((!this->_TXEnabledB) ||
		this->_InStandby ||
		!(this->_SRB & SR_TXRDY_BIT) ||
		!(this->_SRB & SR_TXEMT_BIT))
		return false;

	//Set THRB value to be transmitted
	this->_THRB = b;

	//Update status to indicate that THRA is no longer empty
	this->_SRB &= ~(SR_TXEMT_BIT | SR_TXRDY_BIT);
	this->_SetISRWithSideEffects(this->_ISR & ~ISR_TXRDYB_BIT);

	return false;
}

//We aren't emulating the counters, so IVR is effectively currently general purpose regs
bool XR88C681::Write_IVR(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	return ((XR88C681*)this_ptr)->IntWrite_IVR(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_IVR(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {

	this->_IVR = b;

	return true;
}

//We don't handle emulating the output ports
bool XR88C681::Write_OPCR(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	return ((XR88C681*)this_ptr)->IntWrite_OPCR(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_OPCR(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {

	this->_OPCR = b;

	return true;
}

//Set output port bits command -- we don't emulate the output port, so this currently does nothing
bool XR88C681::Write_SOPBC(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	return ((XR88C681*)this_ptr)->IntWrite_SOPBC(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_SOPBC(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {

	return true;
}

//Clear output port bits command -- we don't emulate the output port, so this currently does nothing
bool XR88C681::Write_COPBC(Device* this_ptr, word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {
	return ((XR88C681*)this_ptr)->IntWrite_COPBC(address, timestamp, triggered_range, b);
}

bool XR88C681::IntWrite_COPBC(word32 address, word32 timestamp, ResponseRange* triggered_range, byte b) {

	return true;
}
