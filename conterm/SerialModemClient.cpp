#include <stdio.h>
#include <Windows.h>
#include "SerialModemClient.h"

SerialModemClient::SerialModemClient()
{
	this->inBuffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);
	this->outBuffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1024);
}

SerialModemClient::~SerialModemClient()
{
	if (this->inBuffer) {
		HeapFree(GetProcessHeap(), 0, this->inBuffer);
	}
	if (this->outBuffer) {
		HeapFree(GetProcessHeap(), 0, this->outBuffer);
	}
}

int SerialModemClient::term_connect(const char* parm1, const char* parm2)
{
	// open a com port
	this->hSerialPort = CreateFileA(parm1,
		GENERIC_READ | GENERIC_WRITE, 
		0, 
		NULL, 
		OPEN_EXISTING,
		NULL, 
		NULL);
	if (this->hSerialPort == INVALID_HANDLE_VALUE)
	{
		printf("FATAL ERROR: Failed to open %s. ERROR %i\n", parm1, GetLastError());
		return 1;
	}

	DCB dcbSerialParams = { 0 };
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

	BOOL success = GetCommState(this->hSerialPort, &dcbSerialParams);
	if (success) {
		dcbSerialParams.BaudRate = CBR_56000;
		dcbSerialParams.ByteSize = 8;
		dcbSerialParams.Parity = NOPARITY;
		dcbSerialParams.StopBits = ONESTOPBIT;
		success = SetCommState(this->hSerialPort, &dcbSerialParams);
		if (0 == success) {
			printf("FATAL ERROR: Failed to set comm state\n");
			CloseHandle(this->hSerialPort);
			return 1;
		}
	}
	else {
		printf("FATAL ERROR: Failed to get comm state\n");
		CloseHandle(this->hSerialPort);
		return 1;
	}

	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = MAXDWORD; // in milliseconds
	timeouts.ReadTotalTimeoutConstant = 10; // in milliseconds
	timeouts.ReadTotalTimeoutMultiplier = 0; // in milliseconds
	timeouts.WriteTotalTimeoutConstant = 0; // in milliseconds
	timeouts.WriteTotalTimeoutMultiplier = 5000; // in milliseconds
	success = SetCommTimeouts(this->hSerialPort, &timeouts);
	if (0 == success) {
		printf("FATAL ERROR: Failed to set comm timetouts\n");
		return 1;
	}

	// DONT DIAL FOR NOW. TESTING!
	DWORD nbw = 0;
	char phbuf[32];
	memset(phbuf, 0, 32);
	sprintf_s(phbuf, 32, "ATDT%s\r", parm2);
	WriteFile(this->hSerialPort, phbuf, (DWORD)strlen(phbuf), &nbw, NULL);

	return 0;
}

int SerialModemClient::term_disconnect()
{
	// DONT HANG UP FOR NOW
	DWORD nbw = 0;
	//CancelIo(this->hSerialPort);
	//SetCommMask(this->hSerialPort, 0);
	WriteFile(this->hSerialPort, "ATH0\r", 5, &nbw, NULL);
	if (this->hSerialPort != INVALID_HANDLE_VALUE)
	{
		CloseHandle(this->hSerialPort);
	}
	return 0;
}

void SerialModemClient::term_requestCharsToRead()
{

}

unsigned int SerialModemClient::term_readChars()
{
	DWORD nbr = 0;
	DWORD dwEventMask = 0;

	// clear the entire buffer
	memset(this->inBuffer, 0, 1024);

	//SetCommMask(this->hSerialPort, EV_RXCHAR);

	//printf("SMC: WAITING FOR COMM EVENT\n");
	//BOOL result = WaitCommEvent(this->hSerialPort, &dwEventMask, NULL);
	//if (TRUE == result) {
		//printf("SMC: READING CHARS\n");
		if (FALSE == ReadFile(this->hSerialPort, this->inBuffer, 1024, &nbr, NULL))
		{
			printf("SMC: FAILED TO READ CHARS\n");
			nbr = 0;
		}
		//else {
			//printf("SMG: Readfile succeeded %i\n", nbr);
		//}
	//}
	//else {
	//	printf("SMC: WAIT COMM EVENT FAILED\n");
	//}
	return nbr;
}

void SerialModemClient::term_writeChars(const char* data, unsigned int nchars)
{
	// add chars to a buffer
	//memset(this->outBuffer, 0, 1024);
	//strncpy_s(this->outBuffer, 1023, data, nchars);
	//unsigned int charsToSend = nchars > 1023 ? 1023 : nchars;
	DWORD nbs = 0;
	// this should break the waitcommevent
	//SetCommMask(this->hSerialPort, 0);
	//printf("SMC: Write chars\n");
	WriteFile(this->hSerialPort, data, nchars, &nbs, NULL);
	//printf("SMC: Write chars done %i\n", nbs);
}

void SerialModemClient::term_printBuffer(unsigned int nchars)
{
	unsigned char c = 0;
	for (unsigned int i = 0; i < nchars; i++) {
		c = this->inBuffer[i];
		printf("%c", c);
	}
}

void SerialModemClient::term_logRaw(FILE* file, unsigned int nchars)
{
	for (unsigned int i = 0; i < nchars; i++) {
		fprintf(file, "pos %04i hex %02x int %03i char '%c'\n",
			i,
			(unsigned char)this->inBuffer[i],
			(unsigned char)this->inBuffer[i],
			this->inBuffer[i]);
	}
}

