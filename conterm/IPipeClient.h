
#pragma once

class IContermClient
{
public:
	virtual int term_connect(HANDLE hIoCompletionPort) = 0;
	virtual int term_disconnect() = 0;
	virtual void term_requestCharsToRead() = 0;
	virtual void term_readChars() = 0;
	virtual void term_writeChars(const char* data, unsigned int nchars) = 0;
	virtual void term_printBuffer(unsigned int nchars) = 0;
	int term_getOperation()
	{
		return this->operation;
	}
	void term_setOperation(int op)
	{
		this->operation = op;
	}
	static const int OP_WRITE = 1;
	static const int OP_CHECK = 2;
	static const int OP_READ = 3;
private:
	int operation = 0;
protected:
	void* lpContermClientContext = NULL;
};

typedef struct _CONTERM_CLIENT_CONTEXT
{
	OVERLAPPED overlapped;
	IContermClient* lpClient;
} CONTERM_CLIENT_CONTEXT;
