
#include <stdio.h>
#define WINAPI __stdcall

#include <WinSock2.h>
#include <ws2bth.h>
#include <bthsdpdef.h>
#include <BluetoothAPIs.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "irprops.lib")
using namespace std;
int main()
{
	WORD wVersionRequested = 0x202;
	WSADATA m_data;
	if (0 == WSAStartup(wVersionRequested, &m_data))
	{
		// CREATE SOCKET
		SOCKET s = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
		//const DWORD lastError = ::GetLastError();

	   if (s == INVALID_SOCKET) {
		 //printf("Failed to get bluetooth socket! %s\n", GetLastErrorMessage(lastError));
		 printf("Failed to get bluetooth socket! \n");
		 exit(1);
	   }
	   
	   WSAPROTOCOL_INFO protocolInfo;
	   int protocolInfoSize = sizeof(protocolInfo);

		if (0 != getsockopt(s, SOL_SOCKET, SO_PROTOCOL_INFO, (char*)&protocolInfo, &protocolInfoSize)) {
		  exit(1);
		}
		
	   SOCKADDR_BTH address;
	   address.addressFamily = AF_BTH;
	   address.btAddr = 0;
	   address.serviceClassId = GUID_NULL;
	   address.port = BT_PORT_ANY;
	   sockaddr *pAddr = (sockaddr*)&address;
	   
	   // ======================================================
	   // BIND
	   // ======================================================
	   if (0 != bind(s, pAddr, sizeof(SOCKADDR_BTH))) {
		  printf("Bind failed\n");
		  //printf("%s\n", GetLastErrorMessage(GetLastError()));
	   } else {
		  printf("\nBinding Successful....\n");
		  int length = sizeof(SOCKADDR_BTH) ;
		  getsockname(s,(sockaddr*)&address,&length);
		  wprintf (L"Local Bluetooth device is %04x%08x \nServer channel = %d\n", GET_NAP(address.btAddr), GET_SAP(address.btAddr), address.port);
	   }
		int size = sizeof(SOCKADDR_BTH);
		if (0 != getsockname(s, pAddr, &size)) {
			printf("Something failed\n");
			//printf("%s\n", GetLastErrorMessage(GetLastError()));
		}
		if (0 != listen(s, 10))
		{
			printf("Timeout in listen\n");
			//printf("%s\n", GetLastErrorMessage(GetLastError()));
		}

		// ======================================================
		// SET SERVICE
		// ======================================================
		WSAQUERYSET service;
		memset(&service, 0, sizeof(service));
		service.dwSize = sizeof(service);
		service.lpszServiceInstanceName = ("Accelerometer Data...");
		service.lpszComment = ("Pushing data to PC");

		//GUID serviceID = OBEXFileTransferServiceClass_UUID;
        GUID serviceID = "00001101-0000-1000-8000-00805F9B34FB";
		service.lpServiceClassId = &serviceID;
		service.dwNumberOfCsAddrs = 1;
		service.dwNameSpace = NS_BTH;

		CSADDR_INFO csAddr;
		memset(&csAddr, 0, sizeof(csAddr));
		csAddr.LocalAddr.iSockaddrLength = sizeof(SOCKADDR_BTH);
		csAddr.LocalAddr.lpSockaddr = pAddr;
		csAddr.iSocketType = SOCK_STREAM;
		csAddr.iProtocol = BTHPROTO_RFCOMM;
		service.lpcsaBuffer = &csAddr;

		if (0 != WSASetService(&service, RNRSERVICE_REGISTER, 0))
		{
			printf("Service registration failed....");
			//printf("%d\n", GetLastErrorMessage(GetLastError()));
		}
		else
		{    
			printf("\nService registration Successful....\n");
		}
		printf("\nBefore accept.........");
		
		// ======================================================
		// ACCEPT
		// ======================================================
		SOCKADDR_BTH sab2;
		int ilen = sizeof(sab2);
		SOCKET s2 = accept (s,(sockaddr*)&sab2, &ilen);
		if (s2 == INVALID_SOCKET)
		{
		 wprintf (L"Socket bind, error %d\n", WSAGetLastError ());
		}
		wprintf (L"\nConnection came from %04x%08x to channel %d\n",
		GET_NAP(sab2.btAddr), GET_SAP(sab2.btAddr), sab2.port);
		wprintf (L"\nAfter Accept\n");
		
		// ======================================================
		// RECEIVE DATA FROM CLIENT
		// ======================================================
		char buffer[1024] = {0}; 
		memset(buffer, 0, sizeof(buffer));
		int r = recv(s2,(char*)buffer, sizeof(buffer), 0);
		printf("%s\n",buffer);

		 closesocket(s2);
		if (0 != WSASetService(&service, RNRSERVICE_DELETE, 0))
		{
			printf("Something else failed\n");
			//printf("%s\n", GetLastErrorMessage(GetLastError()));
		}
		closesocket(s);
		WSACleanup();
	}
}