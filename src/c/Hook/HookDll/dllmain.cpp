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
SOCKET (WSAAPI *pSocket)(_In_ int af, _In_ int type, _In_ int protocol) = socket;
SOCKET WSAAPI MySocket  (_In_ int af, _In_ int type, _In_ int protocol);

int (WINAPI *pConnect)(_In_ SOCKET s, _In_ const struct sockaddr *name, _In_ int namelen) = connect;
int WINAPI MyConnect  (_In_ SOCKET s, _In_ const struct sockaddr *name, _In_ int namelen);

int (WINAPI *pSend)(_In_ SOCKET s, _In_ const char* buf, _In_ int len, _In_ int flags) = send;
int WINAPI MySend  (_In_ SOCKET s, _In_ const char* buf, _In_ int len, _In_ int flags);

//int (WINAPI *pRecv)(SOCKET s, char* buf, int len, int flags) = recv;
//int WINAPI MyRecv(SOCKET s, char* buf, int len, int flags);

int (WINAPI *pClosesocket)(_In_ SOCKET s) = closesocket;
int WINAPI MyClosesocket(_In_ SOCKET s);

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
	MessageBoxW(NULL, convertCharArrayToLPCWSTR(str), NULL, MB_OK);
}


// =============================================================================
// Global variables
// =============================================================================
SOCKET BTSocket = INVALID_SOCKET;
SOCKET TCPSocket = INVALID_SOCKET;
struct addrinfo *result = NULL;
FILE*  pLogFile;
bool    TCPSocketClosed = false;

// DEBUG - change to true for functional run
bool   BTConnect = true; // if false, using TCP connection

// =============================================================================
// Hooked functions
// =============================================================================
SOCKET WSAAPI MySocket(_In_ int af, _In_ int type, _In_ int protocol)
{
	//MsgBox("HookDll : Entered MySocket");
	fopen_s(&pLogFile, "C:\\Users\\Itay\\Documents\\Log.txt", "a+");

	TCPSocket = pSocket(AF_INET, SOCK_STREAM, 0);
	if (TCPSocket == INVALID_SOCKET) {
		//MsgBox("TCP socket failed with error\n");
		fprintf(pLogFile, "[MySocket]\t TCP socket failed with error\n");
		WSACleanup();
		return -1;
	}
	else 
		//MsgBox("TCP socket succeded");
		fprintf(pLogFile, "[MySocket]\t opened TCP socket number %d\n", TCPSocket);
	
	BTSocket = pSocket(af, type, protocol);
	if (BTSocket == INVALID_SOCKET) {
		//MsgBox("BT socket failed with error\n");
		fprintf(pLogFile, "[MySocket]\t BT socket failed with error\n");
		WSACleanup();
		return -1;
	}
	else 
		//MsgBox("BT socket succeded");
		fprintf(pLogFile, "[MySocket]\t opened BT socket number %d\n", BTSocket);

	fclose(pLogFile);
	return BTSocket;
}

int WINAPI MyConnect(_In_ SOCKET s, _In_ const struct sockaddr *name, _In_ int namelen)
{
	int iResult;	
	struct sockaddr_in server;
	fopen_s(&pLogFile, "C:\\Users\\Itay\\Documents\\Log.txt", "a+");

	//MsgBox("HookDll : Entered MyConnect");

	server.sin_addr.s_addr = inet_addr("132.68.50.102");
	server.sin_family = AF_INET;
	server.sin_port = htons(4020);

	// Try to connect to BTSocket
	iResult = pConnect(BTSocket, name, namelen);
	
	// DEBUG begin
	//iResult = -1;
	// DEBUG end
	if (iResult < 0) {
		fprintf(pLogFile, "[MyConnect]\t BT connect failed, trying to connect with TCP\n");
		BTConnect = false;
		iResult = pConnect(TCPSocket, (struct sockaddr *)&server, sizeof(server));
		if (iResult < 0) {
			pClosesocket(TCPSocket);
			TCPSocketClosed = true;
			//MsgBox("Connect failed with error \n");
			fprintf(pLogFile, "[MyConnect]\t TCP Connect failed\n");
			WSACleanup();
			return -1;
		}
		else {
			fprintf(pLogFile, "[MyConnect]\t TCP Connect succeded\n");
		}
			
	}
	else {
		//MsgBox("connect succeded");
		fprintf(pLogFile, "[MyConnect]\t BT Connect succeded\n");
	}
	fclose(pLogFile);

	return iResult;
}


int WINAPI MySend(SOCKET s, const char* buf, int len, int flags)
{
	int iResult;
	//MsgBox("HookDll : Entered MySend");
	//char* msg = "Message from HookDLL  ";
	fopen_s(&pLogFile, "C:\\Users\\Itay\\Documents\\Log.txt", "a+");
	
	if (!BTConnect) {
		iResult = pSend(TCPSocket, buf, strlen(buf), flags);
		// DEBUG - iResult = pSend(TCPSocket, msg, strlen(msg), flags);
		if (iResult == SOCKET_ERROR) {
			//MsgBox("send failed with error: \n");
			fprintf(pLogFile, "[MySend]\t TCP send failed with error %d\n", iResult);
			TCPSocketClosed = 1;
			pClosesocket(TCPSocket);
			WSACleanup();
			return -1;
		}
		else
			//MsgBox("send succeded");
			fprintf(pLogFile, "[MySend]\t TCP send succeeded. Message = %s, iResult =  %d\n", buf, iResult);
	}
	else {
		iResult = pSend(BTSocket, buf, len, flags);
		if (iResult == SOCKET_ERROR) {
			//MsgBox("send failed with error: \n");
			fprintf(pLogFile, "[MySend]\t BT send failed with error %d\n", iResult);
			TCPSocketClosed = 1;
			pClosesocket(TCPSocket);
			WSACleanup();
			return -1;
		}
		else
			//MsgBox("send succeded");
			fprintf(pLogFile, "[MySend]\t BT send succeeded - %d\n", iResult);
	}
	fclose(pLogFile);
	return iResult;
}

int WINAPI MyClosesocket(_In_ SOCKET s) 
{
	int iResult;
	//MsgBox("HookDll : Entered MyClosesocket");
	fopen_s(&pLogFile, "C:\\Users\\Itay\\Documents\\Log.txt", "a+");

	int res1;
	res1 = TCPSocketClosed? 0 : pClosesocket(TCPSocket); // close socket only if it wasn't closed already
	int res2; 
	res2 = pClosesocket(BTSocket);

	fprintf(pLogFile, "[MyClosesocket]\t Sockets closed - %d, %d (zeros are successes)\n", res1, res2);
	iResult = shutdown(TCPSocket, SD_SEND);
	fclose(pLogFile);
	return res2;
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
		DetourAttach(&(PVOID&)pSocket, MySocket);
		DetourAttach(&(PVOID&)pConnect, MyConnect);
		DetourAttach(&(PVOID&)pSend, MySend);
		DetourAttach(&(PVOID&)pClosesocket, MyClosesocket);
		DetourTransactionCommit();
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)pSocket, MySocket);
		DetourDetach(&(PVOID&)pConnect, MyConnect);
		DetourDetach(&(PVOID&)pSend, MySend);
		DetourDetach(&(PVOID&)pClosesocket, MyClosesocket);
		DetourTransactionCommit();
	}

	
	return TRUE;
}



