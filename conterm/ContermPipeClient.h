#pragma once
#include "IPipeClient.h"

class ContermPipeClient :
	public IContermClient
{
public:
	ContermPipeClient();
	~ContermPipeClient();
	int term_connect();
	int term_disconnect();
	void term_requestCharsToRead();
	unsigned int term_readChars();
	void term_writeChars(const char* data, unsigned int nchars);
	void term_printBuffer(unsigned int nchars);
	void term_logRaw(FILE* file, unsigned int nchars);
private:
	static DWORD WINAPI ServerProc(void* pVoid);
	HANDLE hServerThread = INVALID_HANDLE_VALUE;
	HANDLE hPipeServer = INVALID_HANDLE_VALUE;
	HANDLE hClient = INVALID_HANDLE_VALUE;
	int operation = 0;
	char clientReadBuffer[1024];
	HANDLE hServerReadyEvent = INVALID_HANDLE_VALUE;
};

