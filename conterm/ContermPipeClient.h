#pragma once
#include "IPipeClient.h"

class ContermPipeClient :
    public IPipeClient
{
public:
    ContermPipeClient();
    ~ContermPipeClient();
	int connect(HANDLE hIoCompletionPort);
	int disconnect();
	void requestCharsToRead();
	void readChars();
	void writeChars();
	void printBuffer(unsigned int nchars);
	static const int OP_WRITE = 1;
	static const int OP_CHECK = 2;
	static const int OP_READ = 3;
private:
    static DWORD WINAPI ServerProc(void* pVoid);
    HANDLE hServerThread = INVALID_HANDLE_VALUE;
	HANDLE hPipeServer = INVALID_HANDLE_VALUE;
	HANDLE hClient = INVALID_HANDLE_VALUE;
	int operation = 0;
	char clientReadBuffer[1024];
	HANDLE hServerReadyEvent = INVALID_HANDLE_VALUE;
	void* lpContermPipeClientContext = NULL;
};

typedef struct _CONTERM_PIPE_CLIENT_CONTEXT
{
	OVERLAPPED overlapped;
	ContermPipeClient* lpClient;
} CONTERM_PIPE_CLIENT_CONTEXT;
