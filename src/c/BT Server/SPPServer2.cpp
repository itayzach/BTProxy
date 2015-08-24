#include <WinSock2.h>
#include <ws2bth.h>
#include <bthsdpdef.h>
#include <BluetoothAPIs.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
 

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "irprops.lib")
using namespace std;

#define DEFAULT_BUFLEN 512
int main(int argc, char **argv)
{
	WSADATA wsd;
	SOCKET s, s2;
	SOCKADDR_BTH sab, sab2;
	// NULL_GUID 
	GUID nguiD = SerialPortServiceClass_UUID;
	//given : GUID nguiD = {00000000-0000-0000-0000-000000000000};
	int ilen, iResult;

	// This should be const void * type for non-char data
	char *sendbuf = "Test data from receiver...";
	int recvbuflen = DEFAULT_BUFLEN;
	// Change the type accordingly for non-char data
	char recvbuf[DEFAULT_BUFLEN] = "";

	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)
	{
		printf("Unable to load Winsock! Error code is %d\n", WSAGetLastError());
		return 1;
	}
	else
		printf("WSAStartup() is OK, Winsock lib loaded!\n");

	s = socket (AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if (s == INVALID_SOCKET)
	{
		printf ("Socket creation failed, error %d\n", WSAGetLastError());
		return 1;
	}
	else
		printf ("socket() looks fine!\n");

	memset (&sab, 0, sizeof(sab));
	sab.addressFamily  = AF_BTH;
	// We hardcoded it
	sab.port = BT_PORT_ANY;
	// given: sab.port = 1;

	if (0 != bind(s, (SOCKADDR *) &sab, sizeof(sab)))
	{
		printf ("bind() failed with error code %d\n", WSAGetLastError());
		closesocket (s);
		return 1;
	}
	else
		printf ("bind() looks fine!\n");

	if(listen (s, 5) == 0)
		printf("listen() is OK! Listening for connection...\n");
	else
		printf("listen() failed with error code %d\n", WSAGetLastError());

	for ( ; ; )
	{
		// Get information on the port assigned
		ilen = sizeof(sab2);
		s2 = accept (s, (SOCKADDR *)&sab2, &ilen);
		if (s2 == INVALID_SOCKET)
		{
			printf ("accept() failed with error code %d\n", WSAGetLastError ());
			break;
		}
		else
			printf ("accept(), is OK buddy!\n");

		// Print the info
		printf ("Connection came from %04x%08x to channel %d\n", GET_NAP(sab2.btAddr), GET_SAP(sab2.btAddr), sab2.port);
	}
 
    // Receive until the peer closes the connection
    do {
        iResult = recv(s2, recvbuf, recvbuflen, 0);
        if (iResult > 0)
            printf(" %d Bytes received from sender\n", iResult);
        else if ( iResult == 0 )
            printf("Connection closed by peer!\n");
        else
            printf("recv() failed with error code %d\n", WSAGetLastError());
 
    } while(iResult > 0);
 
    // Echo back the data
    iResult = send(s2, recvbuf, recvbuflen, 0 );
    if (iResult == SOCKET_ERROR)
    {
        printf("send() failed with error code %d\n", WSAGetLastError());
        closesocket(s2);
        WSACleanup();
        return 1;
    }
	else
	{
				printf("send() is OK!\n");
				printf("Bytes Sent: %d\n", iResult);
	}

	if(closesocket(s) == 0)
				printf("closesocket() pretty fine!\n");
	if(WSACleanup () == 0)
				printf("WSACleanup() is OK!\n");
	return 0;
}