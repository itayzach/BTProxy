#include <cstdio>
//#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include "C:\Detours\include\detours.h"

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
SOCKET BTSocket = INVALID_SOCKET;
SOCKET TCPSocket = INVALID_SOCKET;
struct addrinfo *result = NULL;
FILE* pLogFile;

// =============================================================================
// Hooked functions
// =============================================================================
int WINAPI MyWSAStartup(_In_  WORD wVersionRequested, _Out_ LPWSADATA lpWSAData)
{
	int iResult;
	fopen_s(&pLogFile, "C:\\Users\\Itay\\Documents\\Log.txt", "a+");
	fprintf(pLogFile, "HookDll : Entered MyWSAStartup\n");
	fclose(pLogFile);
	//MsgBox("HookDll : Entered MyWSAStartup");

	iResult = pWSAStartup(MAKEWORD(2, 2), lpWSAData);
	if (iResult != 0) {
		//MsgBox("WSAStartup failed with error\n");
		return 1;
	}
	else 
		//MsgBox("WSAStartup succeded");
		fprintf(pLogFile, "WSAStartup succeded\n");

	return iResult;
}

SOCKET WSAAPI MySocket(_In_ int af, _In_ int type, _In_ int protocol)
{
	//MsgBox("HookDll : Entered MySocket");
	fopen_s(&pLogFile, "C:\\Users\\Itay\\Documents\\Log.txt", "a+");
	fprintf(pLogFile, "HookDll : Entered MySocket\n");
	
	
	fprintf(pLogFile, "Before - TCPSocket = %d, BTSocket = %d\n", TCPSocket, BTSocket);

	TCPSocket = pSocket(AF_INET, SOCK_STREAM, 0);
	if (TCPSocket == INVALID_SOCKET) {
		//MsgBox("TCP socket failed with error\n");
		fprintf(pLogFile, "TCP socket failed with error\n");
		pWSACleanup();
		return 1;
	}
	else 
		//MsgBox("TCP socket succeded");
		fprintf(pLogFile, "TCP socket succeded\n");
	
	BTSocket = pSocket(af, type, protocol);
	if (BTSocket == INVALID_SOCKET) {
		//MsgBox("BT socket failed with error\n");
		fprintf(pLogFile, "BT socket failed with error\n");
		pWSACleanup();
		return 1;
	}
	else 
		//MsgBox("BT socket succeded");
		fprintf(pLogFile, "BT socket succeded\n");

	fclose(pLogFile);
	return BTSocket;
}

int WINAPI MyConnect(_In_ SOCKET s, _In_ const struct sockaddr *name, _In_ int namelen)
{
	int iResult;	
	struct sockaddr_in server;
	fopen_s(&pLogFile, "C:\\Users\\Itay\\Documents\\Log.txt", "a+");

	//MsgBox("HookDll : Entered MyConnect");
	fprintf(pLogFile, "HookDll : Entered MyConnect\n");

	server.sin_addr.s_addr = inet_addr("132.68.53.90");
	server.sin_family = AF_INET;
	server.sin_port = htons(4020);
	iResult = pConnect(TCPSocket, (struct sockaddr *)&server, sizeof(server));
	if (iResult < 0) {
		pClosesocket(TCPSocket);
		//MsgBox("Connect failed with error \n");
		fprintf(pLogFile, "Connect failed\n");
		pWSACleanup();
		return 1;
	}
	else
		//MsgBox("connect succeded");
		fprintf(pLogFile, "Connect succeded\n");
	fprintf(pLogFile, "TCPSocket = %d, BTSocket = %d\n", TCPSocket,s);
	fclose(pLogFile);

	return iResult;
}


int WINAPI MySend(SOCKET s, const char* buf, int len, int flags)
{
	int iResult;
	//MsgBox("HookDll : Entered MySend");
	fopen_s(&pLogFile, "C:\\Users\\Itay\\Documents\\Log.txt", "a+");
	fprintf(pLogFile, "HookDll : Entered MySend\n");
	//////////////////////////////////////////////////////
	//SOCKET NewConnectSocket = INVALID_SOCKET;
	//
	//NewConnectSocket = pSocket(AF_INET, SOCK_STREAM, 0);
	//if (NewConnectSocket == INVALID_SOCKET) {
	//	MsgBox("new socket failed with error\n");
	//	pWSACleanup();
	//	return 1;
	//}
	//else MsgBox("new socket succeded");
	//struct sockaddr_in NewServer;
	//NewServer.sin_addr.s_addr = inet_addr("132.68.53.90");
	//NewServer.sin_family = AF_INET;
	//NewServer.sin_port = htons(4020);
	//iResult = pConnect(NewConnectSocket, (struct sockaddr *)&NewServer, sizeof(NewServer));
	//if (iResult < 0) {
	//	pClosesocket(NewConnectSocket);
	//	MsgBox("New Connect failed with error \n");
	//	pWSACleanup();
	//	return 1;
	//}
	//else MsgBox("new connect succeded");

	//iResult = pSend(NewConnectSocket, buf, len, flags);
	//if (iResult == SOCKET_ERROR) {
	//	MsgBox("new send failed with error: \n");
	//	pClosesocket(NewConnectSocket);
	//	pWSACleanup();
	//	return 1;
	//}
	//else MsgBox("new send succeded");


	//////////////////////////////////////////////////////
	
	fprintf(pLogFile, "TCPSocket = %d, BTSocket = %d\n", TCPSocket, s);
	

	iResult = pSend(TCPSocket, "MessagE from BT Client", len, flags);
	if (iResult == SOCKET_ERROR) {
		//MsgBox("send failed with error: \n");
		fprintf(pLogFile, "send failed with error %d\n", iResult);
		pClosesocket(TCPSocket);
		pWSACleanup();
		return 1;
	}
	else
		//MsgBox("send succeded");
		fprintf(pLogFile, "send succeeded - %d\n", iResult);
	fclose(pLogFile);
	return iResult;
}

int WINAPI MyClosesocket(_In_ SOCKET s) 
{
	int iResult;
	//MsgBox("HookDll : Entered MyClosesocket");
	fopen_s(&pLogFile, "C:\\Users\\Itay\\Documents\\Log.txt", "a+");
	fprintf(pLogFile, "HookDll : Entered MyClosesocket\n");

	int res1 = pClosesocket(TCPSocket);
	int res2 = pClosesocket(BTSocket);

	fprintf(pLogFile, "Sockets closed - %d, %d\n", res1, res2);
	iResult = shutdown(TCPSocket, SD_SEND);
	fclose(pLogFile);
	return res2;
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
		//DetourAttach(&(PVOID&)pWSAStartup, MyWSAStartup);
		DetourAttach(&(PVOID&)pSocket, MySocket);
		DetourAttach(&(PVOID&)pConnect, MyConnect);
		DetourAttach(&(PVOID&)pSend, MySend);
		DetourAttach(&(PVOID&)pClosesocket, MyClosesocket);
		//DetourAttach(&(PVOID&)pWSACleanup, MyWSACleanup);
		DetourTransactionCommit();
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		//DetourDetach(&(PVOID&)pWSAStartup, MyWSAStartup);
		DetourDetach(&(PVOID&)pSocket, MySocket);
		DetourDetach(&(PVOID&)pConnect, MyConnect);
		DetourDetach(&(PVOID&)pSend, MySend);
		DetourDetach(&(PVOID&)pClosesocket, MyClosesocket);
		//DetourDetach(&(PVOID&)pWSACleanup, MyWSACleanup);
		DetourTransactionCommit();
	}

	
	return TRUE;
}



