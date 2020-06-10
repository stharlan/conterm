#pragma once
#include "IPipeClient.h"
class SerialModemClient :
    public IContermClient
{
public:
    SerialModemClient();
    ~SerialModemClient();

	int term_connect(const char* parm1, const char* parm2);
	int term_disconnect();
	void term_requestCharsToRead();
	unsigned int term_readChars();
	void term_writeChars(const char* data, unsigned int nchars);
	void term_printBuffer(unsigned int nchars);
	void term_logRaw(FILE* file, unsigned int nchars);

private:
	HANDLE hSerialPort = INVALID_HANDLE_VALUE;
	char* inBuffer = NULL;
	char* outBuffer = NULL;
};

