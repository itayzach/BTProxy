#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

#include <stdio.h>
#include <WinSock2.h>
#include <ws2bth.h>
#include <bthsdpdef.h>
#include <BluetoothAPIs.h>
#include <stdlib.h>
#include <errno.h>
// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

int __cdecl main(int argc, char **argv) 
{
	WORD 			wVersionRequested 		= 0x202;
    WSADATA 		wsaData;
    SOCKET 			LocalSocket 			= INVALID_SOCKET;
	SOCKADDR_BTH    SockAddrBthServer;
	char 			*sendbuf 				= "this is a test";
	int 			iResult;
 
    // Initialize Winsock
    iResult = WSAStartup(wVersionRequested, &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
	printf("Initialized Winsock\n");
	
	// Create a SOCKET for connecting to server
	LocalSocket = socket(	AF_BTH,				//ai_family 
							SOCK_STREAM, 		//ai_socktype
							BTHPROTO_RFCOMM);	//ai_protocol
	if (LocalSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	printf("Created local socket\n");

	
	// Define the server address
	SockAddrBthServer.addressFamily = AF_BTH;
	SockAddrBthServer.btAddr = 0xD0C1B14BEB23; 
	SockAddrBthServer.serviceClassId = SerialPortServiceClass_UUID;
	SockAddrBthServer.port = BT_PORT_ANY;	
	
	printf("Attemping to connect\n");
	// Connect to server
	iResult = connect( LocalSocket,  (struct sockaddr *) &SockAddrBthServer, sizeof(SOCKADDR_BTH));
	if (iResult == SOCKET_ERROR) {
		printf("Connect failed with error: %d\n", WSAGetLastError());
		closesocket(LocalSocket);
		return 1;
	}
	printf("Connected to server. Sending message to server\n");
	
	// Send an initial buffer
    iResult = send( LocalSocket, sendbuf, (int)strlen(sendbuf), 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(LocalSocket);
        WSACleanup();
        return 1;
    }
	printf("Message to server was sent\n");
 
    // // Receive until the peer closes the connection
    // do {

        // iResult = recv(LocalSocket, recvbuf, recvbuflen, 0);
        // if ( iResult > 0 )
            // printf("Bytes received: %d\n", iResult);
        // else if ( iResult == 0 )
            // printf("Connection closed\n");
        // else
            // printf("recv failed with error: %d\n", WSAGetLastError());

    // } while( iResult > 0 );

    // cleanup
	printf("Cleaning up\n");
    closesocket(LocalSocket);
    WSACleanup();

    return 0;
}
		
		