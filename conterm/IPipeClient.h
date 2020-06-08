
#pragma once

class IPipeClient
{
public:
	virtual int connect(HANDLE hIoCompletionPort) = 0;
	virtual int disconnect() = 0;
	virtual void requestCharsToRead() = 0;
	virtual void readChars() = 0;
	virtual void writeChars() = 0;
	virtual void printBuffer(unsigned int nchars) = 0;
	int getOperation()
	{
		return this->operation;
	}
	void setOperation(int op) 
	{
		this->operation = op;
	}
private:
	int operation = 0;
};