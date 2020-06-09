// conterm.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "IPipeClient.h"
#include "ContermPipeClient.h"
#include "TelnetClient.h"

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
HANDLE hInputHandlerStartEvent = INVALID_HANDLE_VALUE;
HANDLE hInputHandlerStopEvent = INVALID_HANDLE_VALUE;
HANDLE hInputHandlerThread = INVALID_HANDLE_VALUE;

DWORD WINAPI WorkerThread(void* parm)
{

	HANDLE hIocp = (HANDLE)parm;

	//printf("THREAD: Worker thread started...\n");

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
		//printf("THREAD: Waiting for packet\n");
		result = GetQueuedCompletionStatus(hIocp, &nbt, &cmplKey, &lpOverlapped, INFINITE);
		//printf("THREAD: Got a packet\n");
		if (TRUE == result) {
			CONTERM_CLIENT_CONTEXT* cpctx = (CONTERM_CLIENT_CONTEXT*)lpOverlapped;
			IContermClient* ctx = cpctx->lpClient;
			//printf("THREAD: Operation is %i\n", ctx->term_getOperation());
			if (ctx->term_getOperation() == IContermClient::OP_WRITE)
			{
				// we've just written something, now read
				ctx->term_setOperation(IContermClient::OP_READ);

				// if com port, do this
				//ctx->setOperation(ContermPipeClient::OP_CHECK);
				//SetCommMask(ctx->hCom, EV_RXCHAR);
				//WaitCommEvent(ctx->hCom, &dwEventMask, &ctx->ovl);

				// if named pipe, do this
				// peek
				// read if available bytes
			}
			else if (ctx->term_getOperation() == IContermClient::OP_CHECK)
			{
				//printf("THREAD: %i bytes ready to read.\n", nbt);
				//memset(ctx->buffer, 0, 256);
				//ctx->op = OP_READ;
				//ReadFile(ctx->hCom, ctx->buffer, nbt, &nbr, &ctx->ovl);
			}
			else if (ctx->term_getOperation() == IContermClient::OP_READ)
			{
				//printf("THREAD: Got a read packet\n");
				if (nbt > 0) {
					ctx->term_printBuffer(nbt);
					//printf("\n");
					//memset(ctx->buffer, 0, 256);
					//ReadFile(ctx->hCom, ctx->buffer, nbt, &nbr, &ctx->ovl);
				}
				ctx->term_readChars();
			}
		}
		else {
			if (lpOverlapped == NULL && GetLastError() == ERROR_ABANDONED_WAIT_0)
			{
				result = FALSE;
			}
			else {
				result = TRUE;
			}
		}
	} while (TRUE == result);

	//printf("THREAD: Worker thread quitting.\n");
	return 0;
}

DWORD WINAPI ConsoleInputHandlerThread(void* pVoid)
{
	SetEvent(hInputHandlerStartEvent);

	BOOL bDone = FALSE;

	IContermClient* pClient = (IContermClient*)pVoid;

	do {
		if(WAIT_TIMEOUT == WaitForSingleObject(GetStdHandle(STD_INPUT_HANDLE), 1000))
		{
			if (WAIT_OBJECT_0 == WaitForSingleObject(hInputHandlerStopEvent, 0))
			{
				bDone = TRUE;
			}
		}
		else {
			DWORD nev = 0;
			GetNumberOfConsoleInputEvents(GetStdHandle(STD_INPUT_HANDLE), &nev);

			for (DWORD e = 0; e < nev; e++) {
				INPUT_RECORD inrec = { 0 };
				DWORD ner = 0;
				ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &inrec, 1, &ner);
				if (inrec.EventType == KEY_EVENT) {
					if (TRUE == inrec.Event.KeyEvent.bKeyDown)
					{
						// send to server
						pClient->term_writeChars(&inrec.Event.KeyEvent.uChar.AsciiChar, 1);
					}
				}
			}
		}
	} while (FALSE == bDone);
	return 0;
}

void StartInputHandler(IContermClient* pClient)
{
	printf("MAIN: Starting input handler\n");
	hInputHandlerStartEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hInputHandlerStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hInputHandlerThread = CreateThread(NULL, 0, ConsoleInputHandlerThread, pClient, 0, NULL);
	WaitForSingleObject(hInputHandlerStartEvent, INFINITE);
	printf("MAIN: Input handler started\n");
}

void StopInputHandler()
{
	printf("MAIN: Stopping input handler\n");
	SetEvent(hInputHandlerStopEvent);
	WaitForSingleObject(hInputHandlerThread, INFINITE);
	CloseHandle(hInputHandlerThread);
	CloseHandle(hInputHandlerStartEvent);
	CloseHandle(hInputHandlerStopEvent);
	printf("MAIN: Input handler stopped\n");
}

BOOL WINAPI HandlerRoutine(
	_In_ DWORD dwCtrlType
)
{
	switch (dwCtrlType) {
	case CTRL_C_EVENT:
		CloseHandle(hIocp);
		return TRUE;
	}
	return FALSE;
}

int main()
{

	WSADATA wsadata = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsadata);

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

	//ContermPipeClient* lpClient = new ContermPipeClient();
	TelnetClient* lpClient = new TelnetClient();
	printf("MAIN: Connecting to server\n");
	lpClient->term_connect(hIocp);
	//printf("MAIN: Specifying terminal\n");
	//lpClient->term_writeChars("TERM_TYPE=ANSI-BBS\r");
	//printf("MAIN: Reading chars from server\n");
	//lpClient->term_readChars();

	StartInputHandler(lpClient);

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

	StopInputHandler();

	printf("MAIN: Closing coms\n");
	lpClient->term_disconnect();
	delete lpClient;

	printf("MAIN: Closing io completion port\n");
	CloseHandle(hIocp);

	WSACleanup();

	printf("MAIN: Done.\n");
	return 0;
}

