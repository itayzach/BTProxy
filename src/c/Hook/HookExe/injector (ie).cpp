#undef UNICODE
#include <windows.h>
#include "C:\Detours\include\detours.h"
#pragma comment(lib,"detours.lib")

int main(int argc, char *argv[])
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;

	if (!DetourCreateProcessWithDllEx("C:\\Program Files (x86)\\Internet Explorer\\iexplore.exe",
		NULL, NULL, NULL, TRUE,
		CREATE_DEFAULT_ERROR_MODE,// | CREATE_SUSPENDED,
		NULL, NULL, &si, &pi,
		"C:\\Users\\Itay\\Documents\\Visual Studio 2013\\Projects\\19.5 try\\HookDll\\Debug\\HookDll.dll", NULL))
		MessageBox(0, "failed", 0, 0);
	else
		MessageBox(0, "success", 0, 0);

	ResumeThread(pi.hThread);

	WaitForSingleObject(pi.hProcess, INFINITE);

	CloseHandle(&si);
	CloseHandle(&pi);

	return EXIT_SUCCESS;
}