// conterm.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
#include "IPipeClient.h"
#include "ContermPipeClient.h"

// thread
// check for data to send
//   send data
// check for incoming data
//   

void sendCommandAndWaitForResponse(HANDLE hCom, const char* cmd)
{
	DWORD nbw = 0;
	DWORD nbr = 0;

	char buffer[256];
	memset(buffer, 0, 256);
	strcpy_s(buffer, 256, cmd);
	strcat_s(buffer, 256, "\r");
	WriteFile(hCom, buffer, strlen(buffer), &nbw, NULL);
	printf(cmd);

	SetCommMask(hCom, EV_RXCHAR);
	DWORD dwEventMask = 0;
	WaitCommEvent(hCom, &dwEventMask, NULL);

	do {
		memset(buffer, 0, 256);
		ReadFile(hCom, buffer, 256, &nbr, NULL);
		for (UINT i = 0; i < nbr; i++) {
			printf("%c", buffer[i]);
		}
	} while (nbr > 0);
	printf("\n");
}

HANDLE hIocp = INVALID_HANDLE_VALUE;
HANDLE hThread = INVALID_HANDLE_VALUE;
HANDLE hWorkerThreadReady = INVALID_HANDLE_VALUE;

DWORD WINAPI WorkerThread(void* parm)
{

	HANDLE hIocp = (HANDLE)parm;

	printf("THREAD: Worker thread started...\n");

	DWORD nbt = 0;
	ULONG_PTR cmplKey = 0;
	OVERLAPPED* lpOverlapped = NULL;
	BOOL result = TRUE;
	DWORD dwEventMask = 0;
	DWORD nbr = 0;

	SetEvent(hWorkerThreadReady);
	do {
		// after ten seconds it will timeout
		// result will be FALSE and it will close down
		result = GetQueuedCompletionStatus(hIocp, &nbt, &cmplKey, &lpOverlapped, 10000);
		if (TRUE == result) {
			printf("THREAD: Got a packet\n");
			CONTERM_PIPE_CLIENT_CONTEXT* cpctx = (CONTERM_PIPE_CLIENT_CONTEXT*)lpOverlapped;
			ContermPipeClient* ctx = cpctx->lpClient;
			printf("THREAD: Operation is %i\n", ctx->getOperation());
			if (ctx->getOperation() == ContermPipeClient::OP_WRITE) 
			{
				// we've just written something, now read
				printf("THREAD: Wrote %i bytes, reading...\n", nbt);

				// if com port, do this
				//ctx->setOperation(ContermPipeClient::OP_CHECK);
				//SetCommMask(ctx->hCom, EV_RXCHAR);
				//WaitCommEvent(ctx->hCom, &dwEventMask, &ctx->ovl);

				// if named pipe, do this
				// peek
				// read if available bytes
			}
			else if (ctx->getOperation() == ContermPipeClient::OP_CHECK)
			{
				//printf("THREAD: %i bytes ready to read.\n", nbt);
				//memset(ctx->buffer, 0, 256);
				//ctx->op = OP_READ;
				//ReadFile(ctx->hCom, ctx->buffer, nbt, &nbr, &ctx->ovl);
			}
			else if (ctx->getOperation() == ContermPipeClient::OP_READ)
			{
				printf("THREAD: Got a read packet\n");
				if (nbt > 0) {
					ctx->printBuffer(nbt);
					printf("\n");
					//memset(ctx->buffer, 0, 256);
					//ReadFile(ctx->hCom, ctx->buffer, nbt, &nbr, &ctx->ovl);
				}
			}
		}
	} while (TRUE == result);

	printf("THREAD: Worker thread quitting.\n");
	return 0;
}

BOOL WINAPI HandlerRoutine(
	_In_ DWORD dwCtrlType
)
{
	switch (dwCtrlType) {
	case CTRL_C_EVENT:

		return TRUE;
	}
	return FALSE;
}

int main()
{

	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	printf("MAIN: Creating io completion port...\n");
	hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
	if (hIocp == NULL) {
		printf("MAIN: ERROR: Failed to create io completion port\n");
		return 0;
	}

	printf("MAIN: Creating worker thread...\n");
	hWorkerThreadReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	hThread = CreateThread(NULL, 0, WorkerThread, hIocp, 0, NULL);
	if (NULL == hThread) {
		printf("MAIN: ERROR: Failed to create worker thread\n");
		CloseHandle(hIocp);
		return 0;
	}
	printf("MAIN: Waiting for worker thread to be ready...\n");
	WaitForSingleObject(hWorkerThreadReady, INFINITE);

	//printf("MAIN: Opening COM port 5...\n");
	//HANDLE hCom = CreateFile(L"COM5", 
	//	GENERIC_READ | GENERIC_WRITE, 
	//	0, 
	//	NULL, 
	//	OPEN_EXISTING,
	//	FILE_FLAG_OVERLAPPED, 
	//	NULL);
	//if (hCom == INVALID_HANDLE_VALUE)
	//{
	//	printf("MAIN: failed to open com port\n");
	//	WaitForSingleObject(hThread, INFINITE);
	//	CloseHandle(hThread);
	//	CloseHandle(hIocp);
	//	return 0;
	//}
	//else {
	//	printf("MAIN: com port is open\n");
	//}

	ContermPipeClient* lpClient = new ContermPipeClient();
	printf("MAIN: Connecting to pipe\n");
	lpClient->connect(hIocp);
	printf("MAIN: Reading chars from pipe\n");
	lpClient->readChars();

	//DCB dcbSerialParams = { 0 };
	//dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	//printf("MAIN: Getting COM state...\n");
	//BOOL success = GetCommState(hCom, &dcbSerialParams);
	//if (success) {
	//	printf("MAIN: Got comm state ok\n");
	//	dcbSerialParams.BaudRate = CBR_56000;
	//	dcbSerialParams.ByteSize = 8;
	//	dcbSerialParams.Parity = NOPARITY;
	//	dcbSerialParams.StopBits = ONESTOPBIT;
	//	success = SetCommState(hCom, &dcbSerialParams);
	//	if (success) {
	//		printf("MAIN: Set comm state ok\n");
	//	}
	//	else {
	//		printf("MAIN: ERROR: Failed to set comm state\n");
	//	}
	//}
	//else {
	//	printf("MAIN: ERROR: Failed to get comm state\n");
	//}

	//COMMTIMEOUTS timeouts = { 0 };
	//timeouts.ReadIntervalTimeout = 50; // in milliseconds
	//timeouts.ReadTotalTimeoutConstant = 50; // in milliseconds
	//timeouts.ReadTotalTimeoutMultiplier = 10; // in milliseconds
	//timeouts.WriteTotalTimeoutConstant = 50; // in milliseconds
	//timeouts.WriteTotalTimeoutMultiplier = 10; // in milliseconds

	//printf("MAIN: Setting COM timeouts...\n");
	//success = SetCommTimeouts(hCom, &timeouts);
	//if (success) {
	//	printf("MAIN: set comm timeouts ok\n");
	//}
	//else {
	//	printf("MAIN: ERROR: Failed to set comm timeouts\n");
	//}

	// LOOP HERE

	//DWORD nbw = 0;
	// E0 is echo off
	//const char* blah = "ATI4\r";
	//WriteFile(hCom, blah, strlen(blah), &nbw, &ctx.ovl);

	//BOOL quit = FALSE;
	//char clBuffer[256];
	//do {
		//memset(clBuffer, 0, 256);
		//fgets(clBuffer, 255, stdin);
	//	int ichar = getc(stdin);
	//	if (strcmp(clBuffer, "qquit\n") == 0) {
	//		printf("MAIN: Quitting...\n");
	//		quit = TRUE;
	//	}
	//	else {
	//		size_t pos = strlen(clBuffer);
	//		clBuffer[pos - 1] = '\r';
	//		clBuffer[pos] = 0;
	//		WriteFile(hCom, clBuffer, strlen(clBuffer), &nbw, &ctx.ovl);
	//	}
	//} while (!quit);

	//sendCommandAndWaitForResponse(hCom, "AT");
	//sendCommandAndWaitForResponse(hCom, "ATDT13162953536");
	//
	//getc(stdin);

	//sendCommandAndWaitForResponse(hCom, "+++ATH0");

	////while (true) {
	////	printf("MAIN: > ");
	////	fgets(buffer, 256, stdin);

	////	WriteFile(hCom, buffer, strlen(buffer), &nbw, NULL);

	////	WaitCommEvent(hCom, &dwEventMask, NULL);

	////	memset(buffer, 0, 256);
	////	ReadFile(hCom, buffer, 256, &nbr, NULL);

	////	printf("MAIN: Response: '%s'\n", buffer);
	////}

	printf("MAIN: Waiting for thread to quit...\n");
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);

	printf("MAIN: Closing com port\n");
	lpClient->disconnect();
	delete lpClient;

	printf("MAIN: Closing io completion port\n");
	CloseHandle(hIocp);

	printf("MAIN: Done.\n");
	return 0;
}

