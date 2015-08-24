#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "4020"
#define MSG_LEN 18

int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    hints;
    char *sendbuf = "Message from TCP Client";
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
	char* IP = "132.68.53.90";
	char* port = "4020";
    
    // // Validate the parameters
    // if (argc != 2) {
        // printf("usage: %s server-name\n", argv[0]);
        // return 1;
    // }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo(IP, port, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

	// =========================================
	// Create a SOCKET for connecting to server
	// =========================================
	ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// =========================================
	// Connect to server.
	// =========================================
	iResult = connect( ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		printf("Connect failed with error : %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

	// =========================================
    // Send an initial buffer
	// =========================================
    iResult = send( ConnectSocket, sendbuf, (int)strlen(sendbuf), 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // // Receive until the peer closes the connection
    // do {

        // iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        // if ( iResult > 0 ){
			// char msg[MSG_LEN+1];
			// int k;
			// for (k=0;k<MSG_LEN;k++){
				// msg[k]=recvbuf[k];
			// }
			// msg[MSG_LEN]=NULL;
			// //printf("I got: %s  from server\n",recvbuf);	///add here
			// printf("I got: %s",msg);
            // printf("Bytes received: %d\n", iResult);
		// }
        // else if ( iResult == 0 )
            // printf("Connection closed\n");
        // else
            // printf("recv failed with error: %d\n", WSAGetLastError());

    // } while( iResult > 0 );

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}