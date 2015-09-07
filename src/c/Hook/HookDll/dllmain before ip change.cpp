#include <cstdio>
//#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include "C:\Detours\include\detours.h"
//#include "C:\Users\Itay\Documents\Visual Studio 2013\Projects\19.5 try\DummyApp\dummyApp.h"

#pragma comment(lib,"detours.lib")
#pragma comment(lib,"ws2_32.lib")


// =============================================================================
// Define hooking pointers
// For each function, save its' address in p<FuncName> in order to call the 
// original function using the pointer.
// =============================================================================
int (WINAPI *pWSAStartup)(_In_  WORD wVersionRequested, _Out_ LPWSADATA lpWSAData) = WSAStartup;
int WINAPI MyWSAStartup(_In_  WORD wVersionRequested, _Out_ LPWSADATA lpWSAData);

SOCKET (WSAAPI *pSocket)(_In_ int af, _In_ int type, _In_ int protocol) = socket;
SOCKET WSAAPI MySocket  (_In_ int af, _In_ int type, _In_ int protocol);

int (WINAPI *pConnect)(_In_ SOCKET s, _In_ const struct sockaddr *name, _In_ int namelen) = connect;
int WINAPI MyConnect  (_In_ SOCKET s, _In_ const struct sockaddr *name, _In_ int namelen);

int (WINAPI *pSend)(_In_ SOCKET s, _In_ const char* buf, _In_ int len, _In_ int flags) = send;
int WINAPI MySend(_In_ SOCKET s, _In_ const char* buf, _In_ int len, _In_ int flags);

//int (WINAPI *pRecv)(SOCKET s, char* buf, int len, int flags) = recv;
//int WINAPI MyRecv(SOCKET s, char* buf, int len, int flags);

int (WINAPI *pClosesocket)(_In_ SOCKET s) = closesocket;
int WINAPI MyClosesocket(_In_ SOCKET s);

int (WINAPI *pWSACleanup)(void) = WSACleanup;
int WINAPI MyWSACleanup(void);

// =============================================================================
// convert const char* to LPCWSTR
// =============================================================================
wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

void MsgBox(char *str) {
	MessageBoxW(NULL, convertCharArrayToLPCWSTR(str), NULL, MB_HELP);
}


// =============================================================================
// Global variables
// ============================================================================
SOCKET ConnectSocket = INVALID_SOCKET;
struct addrinfo *result = NULL;
FILE* pSendLogFile;
FILE* pRecvLogFile;

// =============================================================================
// Hooked functions
// =============================================================================
int WINAPI MyWSAStartup(_In_  WORD wVersionRequested, _Out_ LPWSADATA lpWSAData)
{
	int iResult;

	MsgBox("HookDll : Entered MyWSAStartup");

	iResult = pWSAStartup(MAKEWORD(2, 2), lpWSAData);
	if (iResult != 0) {
		MsgBox("WSAStartup failed with error\n");
		return 1;
	}
	else MsgBox("WSAStartup succeded");

	return iResult;
}

SOCKET WSAAPI MySocket(_In_ int af, _In_ int type, _In_ int protocol)
{
	int iResult;
	char *IP = "132.68.53.90";
	char *port = "4020";
	struct addrinfo hints;
	WSADATA wsaData;

	MsgBox("HookDll : Entered MySocket");

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	
	iResult = getaddrinfo(IP, port, &hints, &result);
	if (iResult != 0) {
		MsgBox("getaddrinfo failed with error\n");
		pWSACleanup();
		return 1;
	}
	else MsgBox("getaddrinfo succeded");

	ConnectSocket = pSocket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		MsgBox("socket failed with error\n");
		pWSACleanup();
		return 1;
	}
	else MsgBox("socket succeded");

	return ConnectSocket;
}

int WINAPI MyConnect(_In_ SOCKET s, _In_ const struct sockaddr *name, _In_ int namelen)
{
	int iResult;
	MsgBox("HookDll : Entered MyConnect");
	iResult = pConnect(s, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		MsgBox("Connect failed with error \n");
		pWSACleanup();
		return 1;
	}
	else MsgBox("connect succeded");

	freeaddrinfo(result);

	if (ConnectSocket == INVALID_SOCKET) {
		MsgBox("Unable to connect to server!\n");
		pWSACleanup();
		return 1;
	}
	else MsgBox("socket is valid");
	return iResult;
}


int WINAPI MySend(SOCKET s, const char* buf, int len, int flags)
{
	int iResult;
	MsgBox("HookDll : Entered MySend");
	//fopen_s(&pSendLogFile, "C:\\Users\\Itay\\Documents\\SendLog.txt", "a+");
	//fprintf(pSendLogFile, "%s\n", buf);
	//fclose(pSendLogFile);

	iResult = pSend(ConnectSocket, "Message from dllmain", len, flags);
	if (iResult == SOCKET_ERROR) {
		MsgBox("send failed with error: \n");
		pClosesocket(ConnectSocket);
		pWSACleanup();
		return 1;
	}
	else MsgBox("send succeded");
	return iResult;
}

int WINAPI MyClosesocket(_In_ SOCKET s) 
{
	int iResult;
	MsgBox("HookDll : Entered MyClosesocket");
	pClosesocket(ConnectSocket);

	iResult = shutdown(ConnectSocket, SD_SEND);
	return 0;
}

int WINAPI MyWSACleanup(void) {
	MsgBox("HookDll : Entered MyWSACleanup");
	return pWSACleanup();
}

extern "C" __declspec(dllexport) void dummy(void){
	return;
}


// =============================================================================
// DllMain
// =============================================================================
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
	if (DetourIsHelperProcess()) {
		return TRUE;
	}

	if (dwReason == DLL_PROCESS_ATTACH) {

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)pWSAStartup, MyWSAStartup);
		DetourAttach(&(PVOID&)pSocket, MySocket);
		DetourAttach(&(PVOID&)pConnect, MyConnect);
		DetourAttach(&(PVOID&)pSend, MySend);
		DetourAttach(&(PVOID&)pClosesocket, MyClosesocket);
		DetourAttach(&(PVOID&)pWSACleanup, MyWSACleanup);
		DetourTransactionCommit();
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)pWSAStartup, MyWSAStartup);
		DetourDetach(&(PVOID&)pSocket, MySocket);
		DetourDetach(&(PVOID&)pConnect, MyConnect);
		DetourDetach(&(PVOID&)pSend, MySend);
		DetourDetach(&(PVOID&)pClosesocket, MyClosesocket);
		DetourDetach(&(PVOID&)pWSACleanup, MyWSACleanup);
		DetourTransactionCommit();
	}
	return TRUE;
}



