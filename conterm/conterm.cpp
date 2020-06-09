// conterm.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "IPipeClient.h"
#include "ContermPipeClient.h"
#include "TelnetClient.h"

#pragma comment(lib, "winmm.lib")

IContermClient* g_client = NULL;

// thread
// check for data to send
//   send data
// check for incoming data
//   

/*
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
*/

//HANDLE hIocp = INVALID_HANDLE_VALUE;
//HANDLE hThread = INVALID_HANDLE_VALUE;
//HANDLE hWorkerThreadReady = INVALID_HANDLE_VALUE;
//HANDLE hInputHandlerStartEvent = INVALID_HANDLE_VALUE;
//HANDLE hInputHandlerStopEvent = INVALID_HANDLE_VALUE;
//HANDLE hInputHandlerThread = INVALID_HANDLE_VALUE;

HANDLE hReadThread = INVALID_HANDLE_VALUE;
HANDLE hReadThreadReadyEvent = INVALID_HANDLE_VALUE;
HANDLE hReadThreadQuitEvent = INVALID_HANDLE_VALUE;
HANDLE hWriteThread = INVALID_HANDLE_VALUE;
HANDLE hWriteThreadReadyEvent = INVALID_HANDLE_VALUE;
HANDLE hWriteThreadQuitEvent = INVALID_HANDLE_VALUE;

/*
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

	FILE* log = NULL;
	DWORD tgt = timeGetTime();
	char filename[64];
	memset(filename, 0, 64);
	sprintf_s(filename, 64, "log%08x.txt", tgt);
	fopen_s(&log, filename, "w");

	SetEvent(hWorkerThreadReady);
	do {
		// after ten seconds it will timeout
		// result will be FALSE and it will close down
		//printf("THREAD: Waiting for packet\n");
		fprintf(log, "Waiting for GetQueuedCompletionStatus...\n");
		result = GetQueuedCompletionStatus(hIocp, &nbt, &cmplKey, &lpOverlapped, INFINITE);
		//printf("THREAD: Got a packet\n");
		fprintf(log, "GetQueuedCompletionStatus returned\n");
		if (TRUE == result) {
			fprintf(log, "GetQueuedCompletionStatus returned 'true'\n");
			if (lpOverlapped == NULL) {
				fprintf(log, "OVERLAPPED is null\n");
			}
			else {
				CONTERM_CLIENT_CONTEXT* cpctx = (CONTERM_CLIENT_CONTEXT*)lpOverlapped;
				IContermClient* ctx = cpctx->lpClient;
				fprintf(log, "Operation is %i\n", ctx->term_getOperation());
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
				}
				else if (ctx->term_getOperation() == IContermClient::OP_READ)
				{
					if (nbt > 0) {
						fprintf(log, "**** %i chars transferred\n", nbt);
						ctx->term_logRaw(log, nbt);
						ctx->term_printBuffer(nbt);
						ctx->term_readChars();
					}
					else {
						// if there are chars to write, write the 
						// chars in the buffer
						// otherwise, post a read
					}
				}
			}
		}
		else {
			fprintf(log, "GetQueuedCompletionStatus returned 'false'...\n");
			if (lpOverlapped == NULL && GetLastError() == ERROR_ABANDONED_WAIT_0)
			{
				result = FALSE;
			}
			else {
				result = TRUE;
			}
		}
	} while (TRUE == result);

	fclose(log);

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
						// check if io is pending
						// if io is pending, save this to a buffer
						// if no io pending, write chars
						//pClient->term_writeChars(&inrec.Event.KeyEvent.uChar.AsciiChar, 1);
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
*/

void resetConsole()
{
	DWORD nbw = 0;
	const char ec[2] = { 0x1b, 'c' };
	WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), ec, 2, &nbw, NULL);
	//char esbuffer[7];
	//esbuffer[0] = 27;
	//esbuffer[1] = '[';
	//esbuffer[2] = '0';
	//esbuffer[3] = ';';
	//esbuffer[4] = '3';
	//esbuffer[5] = '1';
	//esbuffer[6] = 'm';
}

void enableANSIEscapeSequences()
{
	DWORD origMode = 0;
	BOOL bRes = 0;

	// enable the console to handle ansi escape sequences
	HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleMode(stdOut, &origMode);
	printf("Console needs virtual terminal processing. Setting...\n");
	bRes = SetConsoleMode(stdOut, origMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
	if (bRes == 0) {
		printf("ERROR: SetConsoleMode failed %i\n", GetLastError());
	}
	else {
		printf("Virtual processing is set.\n");
		resetConsole();
	}
}

// thread to write to server
DWORD WINAPI WriteThread(void* pVoid)
{
	IContermClient* lpClient = (IContermClient*)pVoid;
	SetEvent(hWriteThreadReadyEvent);
	do {
		if (WAIT_TIMEOUT != WaitForSingleObject(GetStdHandle(STD_INPUT_HANDLE), 500))
		{
			DWORD nev = 0;
			GetNumberOfConsoleInputEvents(GetStdHandle(STD_INPUT_HANDLE), &nev);

			if (nev > 0) {
				for (DWORD e = 0; e < nev; e++) {
					INPUT_RECORD inrec = { 0 };
					DWORD ner = 0;
					ReadConsoleInput(GetStdHandle(STD_INPUT_HANDLE), &inrec, 1, &ner);
					if (inrec.EventType == KEY_EVENT) {
						if (TRUE == inrec.Event.KeyEvent.bKeyDown)
						{
							// check if io is pending
							// if io is pending, save this to a buffer
							// if no io pending, write chars
							lpClient->term_writeChars(&inrec.Event.KeyEvent.uChar.AsciiChar, 1);
						}
					}
				}
			}
		}
	} while (WAIT_OBJECT_0 != WaitForSingleObject(hWriteThreadQuitEvent, 0));
	return 0;
}

void StartWriteThread(IContermClient* pClient)
{
	hWriteThreadReadyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hWriteThreadQuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hWriteThread = CreateThread(NULL, 0, WriteThread, (LPVOID)pClient, 0, NULL);
	WaitForSingleObject(hWriteThreadReadyEvent, INFINITE);
}

void StopWriteThread()
{
	SetEvent(hWriteThreadQuitEvent);
	WaitForSingleObject(hWriteThread, INFINITE);
	CloseHandle(hWriteThreadReadyEvent);
	CloseHandle(hWriteThreadQuitEvent);
	CloseHandle(hWriteThread);
}

// thread to read from server
DWORD WINAPI ReadThread(void* pVoid)
{
	IContermClient* lpClient = (IContermClient*)pVoid;

	FILE* log = NULL;
	DWORD tgt = timeGetTime();
	char filename[64];
	memset(filename, 0, 64);
	sprintf_s(filename, 64, "log%08x.txt", tgt);
	fopen_s(&log, filename, "w");

	// signal ready
	SetEvent(hReadThreadReadyEvent);

	do {
		// this will block
		fprintf(log, "Waiting for data...\n");
		unsigned int nbr = lpClient->term_readChars();
		fprintf(log, "Got %i chars.\n", nbr);
		if (nbr > 0) {
			lpClient->term_logRaw(log, nbr);
			lpClient->term_printBuffer(nbr);
		}
	} while (WAIT_OBJECT_0 != WaitForSingleObject(hReadThreadQuitEvent, 0));

	fclose(log);

	return 0;
}

void StartReadThread(IContermClient* pClient)
{
	hReadThreadReadyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hReadThreadQuitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	hReadThread = CreateThread(NULL, 0, ReadThread, (LPVOID)pClient, 0, NULL);
	WaitForSingleObject(hReadThreadReadyEvent, INFINITE);
}

void SignalReadThreadToQuit()
{
	SetEvent(hReadThreadQuitEvent);
}

void WaitForReadThreadToQuit()
{
	WaitForSingleObject(hReadThread, INFINITE);
	CloseHandle(hReadThreadReadyEvent);
	CloseHandle(hReadThreadQuitEvent);
	CloseHandle(hReadThread);
}

BOOL WINAPI HandlerRoutine(
	_In_ DWORD dwCtrlType
)
{
	switch (dwCtrlType) {
	case CTRL_C_EVENT:
		StopWriteThread();
		SignalReadThreadToQuit();
		g_client->term_disconnect();
		return TRUE;
	}
	return FALSE;
}

int main()
{

	WSADATA wsadata = { 0 };
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	enableANSIEscapeSequences();

	//printf("MAIN: Creating io completion port...\n");
	//hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 1);
	//if (hIocp == NULL) {
		//printf("MAIN: ERROR: Failed to create io completion port\n");
		//return 0;
	//}

	//printf("MAIN: Creating worker thread...\n");
	//hWorkerThreadReady = CreateEvent(NULL, TRUE, FALSE, NULL);
	//hThread = CreateThread(NULL, 0, WorkerThread, hIocp, 0, NULL);
	//if (NULL == hThread) {
		//printf("MAIN: ERROR: Failed to create worker thread\n");
		//CloseHandle(hIocp);
		//return 0;
	//}
	//printf("MAIN: Waiting for worker thread to be ready...\n");
	//WaitForSingleObject(hWorkerThreadReady, INFINITE);

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
	g_client = (IContermClient*)lpClient;

	printf("MAIN: Connecting to server\n");
	lpClient->term_connect();

	// start read thread
	printf("MAIN: Starting read thread...\n");
	StartReadThread(lpClient);
	StartWriteThread(lpClient);

	// pressing ctrl-c will close the 
	// client connection and signal
	// the read thread that it can exit
	printf("MAIN: Wating for read thread to quit...\n");
	WaitForReadThreadToQuit();

	// disconnect client();
	delete lpClient;
	lpClient = NULL;

	//printf("MAIN: Specifying terminal\n");
	//lpClient->term_writeChars("TERM_TYPE=ANSI-BBS\r");
	//printf("MAIN: Reading chars from server\n");
	//lpClient->term_readChars();

	//StartInputHandler(lpClient);

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


	//printf("MAIN: Waiting for thread to quit...\n");
	//WaitForSingleObject(hThread, INFINITE);
	//CloseHandle(hThread);

	//StopInputHandler();

	//printf("MAIN: Closing io completion port\n");
	//CloseHandle(hIocp);

	WSACleanup();

	resetConsole();

	printf("MAIN: Done.\n");
	return 0;
}

