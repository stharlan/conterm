#pragma once
#include "IPipeClient.h"
class TelnetClient :
    public IContermClient
{
public:
	TelnetClient();
	~TelnetClient();

	int term_connect();
	int term_disconnect();
	void term_requestCharsToRead();
	unsigned int term_readChars();
	void term_writeChars(const char* data, unsigned int nchars);
	void term_printBuffer(unsigned int nchars);
	void term_logRaw(FILE* file, unsigned int nchars);
private:
	unsigned int parseTelnetCommand(unsigned int pos, unsigned int nchars);
	unsigned int parseAnsiEscape(unsigned int pos, unsigned int nchars);

	SOCKET ConnectSocket = INVALID_SOCKET;
	WSABUF inWsaBuffer;
	char* inBuffer;
	WSABUF outWsaBuffer;
	char* outBuffer;
	unsigned char telnetCommand[5];
	int commandMode = 0;
	HANDLE hConsoleOut = INVALID_HANDLE_VALUE;
};

