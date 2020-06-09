
#include <Windows.h>
#include <stdio.h>
#include "ContermPipeClient.h"

const char* THIS_PIPE_NAME = "\\\\.\\pipe\\conterm_pipe_client";

ContermPipeClient::ContermPipeClient()
{
    printf("CONTERM: Creating conterm pipe client\n");

    memset(this->clientReadBuffer, 0, 1024);

    //CONTERM_CLIENT_CONTEXT* lpContext = new CONTERM_CLIENT_CONTEXT();
    //lpContext->lpClient = (IContermClient*)this;
    //memset(&lpContext->overlapped, 0, sizeof(OVERLAPPED));
    //this->lpContermClientContext = (void*)lpContext;

    this->hServerReadyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    this->hServerThread = CreateThread(NULL, 0,
        ContermPipeClient::ServerProc, (LPVOID)this, 0, NULL);
    WaitForSingleObject(this->hServerReadyEvent, INFINITE);
}

ContermPipeClient::~ContermPipeClient()
{
    // signal server to quit
    CloseHandle(this->hPipeServer);
    if (hServerThread) {
        WaitForSingleObject(this->hServerThread, INFINITE);
        CloseHandle(this->hServerThread);
        this->hServerThread = NULL;
    }
    this->hPipeServer = NULL;
    //CONTERM_CLIENT_CONTEXT* lpContext = (CONTERM_CLIENT_CONTEXT*)this->lpContermClientContext;
    //delete lpContext;
    //this->lpContermClientContext = NULL;
}

DWORD WINAPI ContermPipeClient::ServerProc(void* pVoid)
{
    printf("CONTERM: Starting conterm pipe client server proc thread\n");

    ContermPipeClient* lpClient = (ContermPipeClient*)pVoid;
    char buffer[1024];

    printf("CONTERM: Creating named pipe\n");
    lpClient->hPipeServer = CreateNamedPipeA(THIS_PIPE_NAME,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE|PIPE_READMODE_BYTE|PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        1024,
        1024,
        0,
        NULL);
    if (lpClient->hPipeServer == INVALID_HANDLE_VALUE)
    {
        printf("Failed to create named pipe\n");
    }

    // blocking - wait for connection
    printf("CONTERM: Waiting for pipe connection\n");
    SetEvent(lpClient->hServerReadyEvent);
    ConnectNamedPipe(lpClient->hPipeServer, NULL);

    // write the initial menu here
    const char* WelcomeMessage = "Hello, from the named pipe!\r";
    DWORD nbw = 0;
    DWORD nbr = 0;
    printf("CONTERM: Sending hello message on pipe\n");
    WriteFile(lpClient->hPipeServer, WelcomeMessage, strlen(WelcomeMessage), &nbw, NULL);

    BOOL status = FALSE;

    printf("CONTERM: Starting server pipe loop\n");
    do {
        status = ReadFile(lpClient->hPipeServer, buffer, 1, &nbr, NULL);
        // echo it back
        if (TRUE == status && nbr == 1) {
            status = WriteFile(lpClient->hPipeServer, buffer, 1, &nbw, NULL);
        }
    } while (TRUE == status);

    printf("CONTERM: Closing server pipe handle\n");
    if (lpClient->hPipeServer) {
        CloseHandle(lpClient->hPipeServer);
    }

    printf("CONTERM: Server pipe thread done\n");
    return 0;
}


int ContermPipeClient::term_connect()
{
    //this->hClient = CreateFileA(THIS_PIPE_NAME,
    //    GENERIC_READ | GENERIC_WRITE,
    //    0,
    //    NULL,
    //    OPEN_EXISTING,
    //    FILE_FLAG_OVERLAPPED,
    //    NULL);
    //CreateIoCompletionPort(this->hClient, hIoCompletionPort, (ULONG_PTR)this, 0);
    return 0;
}

int ContermPipeClient::term_disconnect()
{
    CloseHandle(this->hClient);
    return 0;
}

void ContermPipeClient::term_requestCharsToRead()
{

}

void ContermPipeClient::term_logRaw(FILE* file, unsigned int nchars)
{

}

unsigned int ContermPipeClient::term_readChars()
{
    //this->term_setOperation(OP_READ);
    //memset(this->clientReadBuffer, 0, 1024);
    //DWORD nbr = 0;
    //CONTERM_CLIENT_CONTEXT* lpContext = (CONTERM_CLIENT_CONTEXT*)this->lpContermClientContext;
    //ReadFile(this->hClient, this->clientReadBuffer, 1024,
    //    &nbr, &lpContext->overlapped);
    return 0;
}

void ContermPipeClient::term_writeChars(const char* data, unsigned int nchars)
{
    //this->term_setOperation(OP_WRITE);
}

void ContermPipeClient::term_printBuffer(unsigned int nchars)
{
    for (unsigned int i = 0; i < nchars; i++) {
        printf("%c", this->clientReadBuffer[i]);
    }
}