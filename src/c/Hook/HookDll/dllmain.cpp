#include <cstdio>
#include <windows.h>

#include "C:\Detours\include\detours.h"
#include "C:\Users\Itay\Documents\Visual Studio 2013\Projects\19.5 try\DummyApp\dummyApp.h"

#pragma comment(lib,"detours.lib")
#pragma comment(lib,"ws2_32.lib")

int (WINAPI *pSend)(SOCKET s, const char* buf, int len, int flags) = send;
int WINAPI MySend(SOCKET s, const char* buf, int len, int flags);
int (WINAPI *pRecv)(SOCKET s, char* buf, int len, int flags) = recv;
int WINAPI MyRecv(SOCKET s, char* buf, int len, int flags);

int WINAPI MyMessageBox(
	_In_opt_ HWND    hWnd,
	_In_opt_ LPCTSTR lpText,
	_In_opt_ LPCTSTR lpCaption,
	_In_     UINT    uType
	);

int (WINAPI *pMessageBox)(
	_In_opt_ HWND    hWnd,
	_In_opt_ LPCTSTR lpText,
	_In_opt_ LPCTSTR lpCaption,
	_In_     UINT    uType
	) = MessageBoxW;

//int(*pPopUp)(void);
//int myPopUp(void);


// convert const char* to LPCWSTR
wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

FILE* pSendLogFile;
FILE* pRecvLogFile;

//int myPopUp(void) {
//	MessageBoxW(NULL, convertCharArrayToLPCWSTR("HookDll : MyPopUp!"), NULL, MB_HELP);
//	return 1;
//}

int WINAPI MyMessageBox(
	_In_opt_ HWND    hWnd,
	_In_opt_ LPCTSTR lpText,
	_In_opt_ LPCTSTR lpCaption,
	_In_     UINT    uType
	) {
		pMessageBox(NULL, convertCharArrayToLPCWSTR("HookDll : Entered MyMessageBox"), NULL, MB_HELP);
		OutputDebugString(convertCharArrayToLPCWSTR("HookDll : Entered MyMessageBox"));
		return pMessageBox(hWnd, lpText, lpCaption, uType);
}

int WINAPI MySend(SOCKET s, const char* buf, int len, int flags)
{
	MessageBoxW(NULL, convertCharArrayToLPCWSTR("HookDll : Entered MySend"), NULL, MB_HELP);
	fopen_s(&pSendLogFile, "C:\\Users\\Itay\\Documents\\SendLog.txt", "a+");
	fprintf(pSendLogFile, "%s\n", buf);
	fclose(pSendLogFile);
	return pSend(s, buf, len, flags);
}

int WINAPI MyRecv(SOCKET s, char* buf, int len, int flags)
{
	MessageBoxW(NULL, convertCharArrayToLPCWSTR("HookDll : Entered MyRecv"), NULL, MB_HELP);
	fopen_s(&pRecvLogFile, "C:\\Users\\Itay\\Documents\\RecvLog.txt", "a+");
	fprintf(pRecvLogFile, "%s\n", buf);
	fclose(pRecvLogFile);
	return pRecv(s, buf, len, flags);
}

extern "C" __declspec(dllexport) void dummy(void){
	return;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
	if (DetourIsHelperProcess()) {
		return TRUE;
	}

	if (dwReason == DLL_PROCESS_ATTACH) {
		//DetourRestoreAfterWith();

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)pMessageBox, MyMessageBox);
		DetourTransactionCommit();

		//DetourTransactionBegin();
		//DetourUpdateThread(GetCurrentThread());
		//DetourAttach(&(PVOID&)pRecv, MyRecv);
		//DetourTransactionCommit();
		//MessageBoxW(NULL, convertCharArrayToLPCWSTR("HookDll : Finished attach"), NULL, MB_HELP);
	}
	else if (dwReason == DLL_PROCESS_DETACH) {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)pMessageBox, MyMessageBox);
		DetourTransactionCommit();

		//DetourTransactionBegin();
		//DetourUpdateThread(GetCurrentThread());
		//DetourDetach(&(PVOID&)pRecv, MyRecv);
		//DetourTransactionCommit();
		//MessageBoxW(NULL, convertCharArrayToLPCWSTR("HookDll : Finished detach"), NULL, MB_HELP);
	}
	return TRUE;
}



