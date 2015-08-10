#include "dummyApp.h"

// convert const char* to LPCWSTR
wchar_t *convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}

int popUp(void) {
	MessageBoxW(0, convertCharArrayToLPCWSTR("DummyApp popup"), 0, 0);
	return 1;
}

int main() {
	
	popUp();
	MessageBoxW(0, convertCharArrayToLPCWSTR("main() finished"), 0, 0);

	return 0;
}