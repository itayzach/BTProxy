#include <winsock2.h>
#include <ws2bth.h>
#include <BluetoothAPIs.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "irprops.lib")
using namespace std;


int main(){

	//Initialising winsock
	WSADATA data;
	int result;
	result = WSAStartup(MAKEWORD(2, 2), &data);//initializing winsock
	if (result!=0){
		cout << "An error occured while initialising winsock, closing....";
		exit(result);
	}

	//Initialising query for device
	WSAQUERYSET queryset;
	memset(&queryset, 0, sizeof(WSAQUERYSET));
	queryset.dwSize = sizeof(WSAQUERYSET);
	queryset.dwNameSpace = NS_BTH;

	HANDLE hLookup;
	result = WSALookupServiceBegin(&queryset, LUP_CONTAINERS, &hLookup);
	if (result!=0){
		cout << "An error occured while initialising look for devices, closing....";
	exit(result);
	}

	//Initialisation succeed, start looking for devices
	BYTE buffer[4096];
	memset(buffer, 0, sizeof(buffer));
	DWORD bufferLength = sizeof(buffer);
	WSAQUERYSET *pResults = (WSAQUERYSET*)&buffer;
	while (result==0){
		result = WSALookupServiceNext(hLookup, 
			LUP_RETURN_NAME | LUP_CONTAINERS  | LUP_RETURN_ADDR | LUP_FLUSHCACHE | 
			LUP_RETURN_TYPE | LUP_RETURN_BLOB | LUP_RES_SERVICE,
			&bufferLength, pResults);
		if(result==0){// A device found
			//print the name of the device
			LPTSTR s = pResults->lpszServiceInstanceName;
			wcout << s << "found. \n";

		}
	}
    return 0;     

}


