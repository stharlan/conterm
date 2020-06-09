#pragma once
#include "IPipeClient.h"
class TelnetClient :
    public IContermClient
{
public:
	TelnetClient();
	~TelnetClient();

	int term_connect(HANDLE hIoCompletionPort);
	int term_disconnect();
	void term_requestCharsToRead();
	void term_readChars();
	void term_writeChars(const char* data, unsigned int nchars);
	void term_printBuffer(unsigned int nchars);

private:
	unsigned int parseTelnetCommand(unsigned int pos, unsigned int nchars);
	unsigned int parseAnsiEscape(unsigned int pos, unsigned int nchars);

	WSABUF clientWsaBuffer;
	SOCKET ConnectSocket = INVALID_SOCKET;
	char* lpBuffer;
	unsigned char telnetCommand[5];
	int commandMode = 0;
};

