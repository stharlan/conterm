#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "TelnetClient.h"

#pragma comment(lib, "Ws2_32.lib")

TelnetClient::TelnetClient()
{
    //CONTERM_CLIENT_CONTEXT* lpContext = new CONTERM_CLIENT_CONTEXT();
    //lpContext->lpClient = this;
    //memset(&lpContext->overlapped, 0, sizeof(OVERLAPPED));
    //this->lpContermClientContext = (void*)lpContext;

    HANDLE hHeap = GetProcessHeap();
    this->inBuffer = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 1024);
    this->inWsaBuffer.buf = this->inBuffer;
    this->inWsaBuffer.len = 1024;

    this->outBuffer = (char*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 1024);
    this->outWsaBuffer.buf = this->outBuffer;
    this->outWsaBuffer.len = 1024;

    this->commandMode = 0;
    memset(this->telnetCommand, 0, 5);

    this->hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
}

TelnetClient::~TelnetClient()
{
    HANDLE hHeap = GetProcessHeap();
    HeapFree(hHeap, 0, this->inBuffer);
    HeapFree(hHeap, 0, this->outBuffer);

    //CONTERM_CLIENT_CONTEXT* lpContext = (CONTERM_CLIENT_CONTEXT*)this->lpContermClientContext;
    //delete lpContext;
    //this->lpContermClientContext = NULL;
}

// parm1 is host
// parm2 is port
int TelnetClient::term_connect(const char* parm1, const char* parm2)
{
    struct addrinfo* result = NULL;
    struct addrinfo* ptr = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    
    // Resolve the server address and port
    int iResult = getaddrinfo(parm1, parm2, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed: %d\n", iResult);
        return 1;
    }

    // Attempt to connect to the first address returned by
    // the call to getaddrinfo
    ptr = result;

    // Create a SOCKET for connecting to server
    this->ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
        ptr->ai_protocol);

    if (this->ConnectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        return 1;
    }

    // Connect to server.
    //printf("TELNET: Connecting to socket\n");
    iResult = connect(this->ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        //printf("TELNET: Failed to connect to socket\n");
        if (WSAGetLastError() == WSAECONNREFUSED) {
            printf("ERROR: No connection could be made because the target machine actively refused it.\n");
        }
        else {
            printf("CONNECT ERROR: %i\n", WSAGetLastError());
        }
        closesocket(this->ConnectSocket);
        this->ConnectSocket = INVALID_SOCKET;
        return 1;
    }

    // Should really try the next address returned by getaddrinfo
    // if the connect call failed
    // But for this simple example we just free the resources
    // returned by getaddrinfo and print an error message

    freeaddrinfo(result);

    if (this->ConnectSocket == INVALID_SOCKET) {
        //printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }
    //else {
        //printf("TELNET: Associating socket with iocp\n");
        //CreateIoCompletionPort((HANDLE)this->ConnectSocket, hIoCompletionPort, (ULONG_PTR)this, 0);
    //}

    // begin receiving immediately
    //this->term_readChars();

    return 0;
}

int TelnetClient::term_disconnect()
{
    if (this->ConnectSocket) {
        closesocket(this->ConnectSocket);
    }
	return 0;
}

void TelnetClient::term_requestCharsToRead()
{

}

unsigned int TelnetClient::term_readChars()
{

    DWORD nbr = 0;
    DWORD flags = 0;

    //printf("TELNET: Reading chars\n");
    //this->term_setOperation(OP_READ);

    // clear the entire buffer
    memset(this->inBuffer, 0, 1024);

    // configure the wsabuf
    this->inWsaBuffer.len = 1024;

    //CONTERM_CLIENT_CONTEXT* lpContext = (CONTERM_CLIENT_CONTEXT*)this->lpContermClientContext;
    //int result = WSARecv(this->ConnectSocket, &this->clientWsaBuffer, 1, &nbr, &flags, &lpContext->overlapped, NULL);
    int result = WSARecv(
        this->ConnectSocket,
        &this->inWsaBuffer,
        1,
        &nbr,
        &flags,
        NULL,
        NULL);
    if(result == SOCKET_ERROR)
    {
        //printf("\nERROR: socket error on recv\n");
        nbr = 0;
    }
    return nbr;
}

void TelnetClient::term_writeChars(const char* data, unsigned int nchars)
{
    //this->term_setOperation(OP_WRITE);
    memset(this->outBuffer, 0, 1024);
    strncpy_s(this->outBuffer, 1023, data, nchars);
    this->outWsaBuffer.len = (nchars > 1024) ? 1024 : nchars;
    DWORD nbs = 0;
    //CONTERM_CLIENT_CONTEXT* lpContext = (CONTERM_CLIENT_CONTEXT*)this->lpContermClientContext;
    WSASend(
        this->ConnectSocket, 
        &this->outWsaBuffer,
        1, 
        &nbs, 
        0, 
        NULL, 
        NULL);
}

enum TELNET_OPT_CODE {
    OPT_BINARY_XMIT = 0,
    OPT_ECHO_DATA = 1,
    OPT_RECONNECT = 2,
    OPT_SUPPRESS_GA = 3,
    OPT_MESSAGE_SZ = 4,
    OPT_OPT_STATUS = 5,
    OPT_TIMING_MARK = 6,
    OPT_R_C_XMTECHO = 7,
    OPT_LINE_WIDTH = 8,
    OPT_PAGE_LENGTH = 9,
    OPT_CR_USE = 10,
    OPT_HORIZ_TABS = 11,
    OPT_HOR_TAB_USE = 12,
    OPT_FF_USE = 13,
    OPT_VERT_TABS = 14,
    OPT_VER_TAB_USE = 15,
    OPT_LF_USE = 16,
    OPT_EXT_ASCII = 17,
    OPT_LOGOUT = 18,
    OPT_BYTE_MACRO = 19,
    OPT_DATA_TERM = 20,
    OPT_SUPDUP = 21,
    OPT_SUPDUP_OUTP = 22,
    OPT_SEND_LOCATE = 23,
    OPT_TERM_TYPE = 24,
    OPT_END_RECORD = 25,
    OPT_TACACS_ID = 26,
    OPT_OUTPUT_MARK = 27,
    OPT_TERM_LOCNUM = 28,
    OPT_3270_REGIME = 29,
    OPT_X_3_PAD = 30,
    OPT_WINDOW_SIZE = 31,
    OPT_TERM_SPEED = 32,
    OPT_REMOTE_FLOW = 33,
    OPT_LINEMODE = 34,
    OPT_X_DISPL_LOC = 35,
    OPT_TELNET_AUTH_OPT = 37,
    OPT_TELNET_ENV_OPT = 39,
    OPT_EXTENDED = 255
};

enum TELNET_CMD_CODE {
    CC_SE = 240,
    CC_NOP = 241,
    CC_DATA_MARK = 242,
    CC_BREAK = 243,
    CC_INT_PROC = 244,
    CC_ABORT_OUTPUT = 245,
    CC_YOU_THERE = 246,
    CC_ERASE_CHAR = 247,
    CC_ERASE_LINE = 248,
    CC_GO_AHEAD = 249,
    CC_SB = 250,
    CC_WILL = 251,
    CC_WONT = 252,
    CC_DO = 253,
    CC_DONT = 254,
    CC_IAC = 255
};

unsigned int TelnetClient::parseTelnetCommand(unsigned int pos, unsigned int nchars)
{
    int charsLeft = nchars - (pos + 1);
    // not enough chars left
    if (charsLeft < 2) return -1;

    printf("** IAC; ");

    unsigned char telnetCmd = this->inBuffer[pos + 1];
    switch (telnetCmd) {
    case CC_SE: printf("END SUBNEG; "); break;
    case CC_NOP: printf("NO OPERATION; "); break;
    case CC_DATA_MARK: printf("DATA MARK; "); break;
    case CC_BREAK: printf("BREAK; "); break;
    case CC_INT_PROC: printf("INT PROCESS; "); break;
    case CC_ABORT_OUTPUT: printf("ABORT OUTPUT; "); break;
    case CC_YOU_THERE: printf("YOU THERE?; "); break;
    case CC_ERASE_CHAR: printf("ERASE CHAR; "); break;
    case CC_ERASE_LINE: printf("ERASE LINE; "); break;
    case CC_GO_AHEAD: printf("GO AHEAD; "); break;
    case CC_SB: printf("SUBNEG; "); break;
    case CC_WILL: printf("WILL USE; "); break;
    case CC_WONT: printf("WON'T USE; "); break;
    case CC_DO: printf("START USE; "); break;
    case CC_DONT: printf("STOP USE; "); break;
    case CC_IAC: printf("IAC; "); break;
    default: 
        printf("UNK CMD %i; ", telnetCmd);
        break; // invalid command
    }

    unsigned char telnetOpt = this->inBuffer[pos + 2];
    if (telnetOpt == OPT_EXTENDED)
    {
        printf("EXT-OPT-LIST; ");
    }
    else {
        switch (telnetOpt) {
        case OPT_BINARY_XMIT: printf("BINARY XMIT; "); break;
        case OPT_ECHO_DATA: printf("ECHO DATA; "); break;
        case OPT_RECONNECT: printf("RECONNECT; "); break;
        case OPT_SUPPRESS_GA: printf("SUPPRESS GA; "); break;
        case OPT_MESSAGE_SZ: printf("MESSAGE SZ; "); break;
        case OPT_OPT_STATUS: printf("OPT STATUS; "); break;
        case OPT_TIMING_MARK: printf("TIMING MARK; "); break;
        case OPT_R_C_XMTECHO: printf("R/C XMT ECHO; "); break;
        case OPT_LINE_WIDTH: printf("LINE WIDTH; "); break;
        case OPT_PAGE_LENGTH: printf("PAGE LENGTH; "); break;
        case OPT_CR_USE: printf("CR USE; "); break;
        case OPT_HORIZ_TABS: printf("HORIZ TABS; "); break;
        case OPT_HOR_TAB_USE: printf("HORIZ TAB USE; "); break;
        case OPT_FF_USE: printf("FF USE; "); break;
        case OPT_VERT_TABS: printf("VERT TABS; "); break;
        case OPT_VER_TAB_USE: printf("VERT TAB USE; "); break;
        case OPT_LF_USE: printf("LF USE; "); break;
        case OPT_EXT_ASCII: printf("EXT ASCII; "); break;
        case OPT_LOGOUT: printf("LOGOUT; "); break;
        case OPT_BYTE_MACRO: printf("BYTE MACRO; "); break;
        case OPT_DATA_TERM: printf("DATA TERM; "); break;
        case OPT_SUPDUP: printf("SUPDUP; "); break;
        case OPT_SUPDUP_OUTP: printf("SUPDUP OUTP; "); break;
        case OPT_SEND_LOCATE: printf("SEND LOCATE; "); break;
        case OPT_TERM_TYPE: printf("TERM TYPE; "); break;
        case OPT_END_RECORD: printf("END RECORD; "); break;
        case OPT_TACACS_ID: printf("TACACS ID; "); break;
        case OPT_OUTPUT_MARK: printf("OUTPUT MARK; "); break;
        case OPT_TERM_LOCNUM: printf("TERM LOC#; "); break;
        case OPT_3270_REGIME: printf("3270 REGIME; "); break;
        case OPT_X_3_PAD: printf("X 3 PAD; "); break;
        case OPT_WINDOW_SIZE: printf("WINDOW SIZE; "); break;
        case OPT_TERM_SPEED: printf("TERM SPEED; "); break;
        case OPT_REMOTE_FLOW: printf("REMOTE FLOW; "); break;
        case OPT_LINEMODE: printf("LINEMODE; "); break;
        case OPT_X_DISPL_LOC: printf("X DISPL LOC; "); break;
        case OPT_TELNET_AUTH_OPT: printf("TELNET AUTH OPT; "); break;
        case OPT_TELNET_ENV_OPT: printf("TELNET ENV OPT; "); break;
        default: printf("UNK OPT %i; ", telnetOpt); break;
        }
    }

    printf("\n");
    return pos + 2;
}

unsigned int TelnetClient::parseAnsiEscape(unsigned int pos, unsigned int nchars)
{
    unsigned int initialPos = pos;
    int charsLeft = nchars - (pos + 1);
    DWORD nbw = 0;

    if (charsLeft < 1) {
        printf("\nreturning early from esacpe sequence\n");
        return pos;
    }
    pos++; charsLeft--;
    //if (this->lpBuffer[pos] != '[')
    if (this->inBuffer[pos] < 0x40 || this->inBuffer[pos] >0x5f)
    {
        printf("\nreturning early from esacpe sequence\n");
        return pos;
    }

    if (charsLeft < 1) {
        printf("\nreturning early from esacpe sequence\n");
        return pos;
    }
    pos++; charsLeft--;
    do {
        if (this->inBuffer[pos] > 0x3f && this->inBuffer[pos] < 0x7f)
        {
            // this is the ending char
            //printf("** ANSI ESCAPE SEQUENCE\n");
            WriteConsoleA(this->hConsoleOut, 
                this->inBuffer + initialPos,
                (pos - initialPos) + 1, &nbw, NULL);
            return pos;
        }
        pos++; charsLeft--;
    } while (charsLeft >= 0);

    // returning early; error?
    printf("\nreturning early from esacpe sequence\n");
    return pos;
}

void TelnetClient::term_logRaw(FILE* file, unsigned int nchars)
{
    for (unsigned int i = 0; i < nchars; i++) {
        fprintf(file, "pos %04i hex %02x int %03i char '%c'\n", 
            i,
            (unsigned char)this->inBuffer[i],
            (unsigned char)this->inBuffer[i],
            this->inBuffer[i]);
    }
}

void TelnetClient::term_printBuffer(unsigned int nchars)
{
    unsigned char c = 0;
    for (unsigned int i = 0; i < nchars; i++) {
        // control 0-31 and 127
        // special 32-47, 58-64, 91-96, 123-126
        // numbers 39-39
        // letters 65-90, 97-122
        c = this->inBuffer[i];
        if (c == 255) {
            // telnet command (do it!)
            i = this->parseTelnetCommand(i, nchars);
        }
        else {
            printf("%c", c);
        }
        //else if (c == 27) {
            // escape!
            //i = this->parseAnsiEscape(i, nchars);
        //}
        //else if (c == 8 || c == 10 || c == 13 || (c > 31 && c < 127)) {
            // printable! ooh!
            //printf("%c", c);
        //}
        //else {
            // other, hmmm...
            //printf("\nVAL: %i\n", c);
        //}
    }
}

