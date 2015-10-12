#define DEFAULT_MSGLEN 512
#define DEFAULT_IDLEN 7 // btproxy / windspc
#define DEFAULT_PORT "4020"

#pragma comment(lib, "ws2_32.lib") //Winsock Library
#include<stdio.h>
#include<winsock2.h>
#include <ws2tcpip.h>
 
int main(int argc , char *argv[])
{
    WSADATA wsaData;
    int iResult, i;
	int MaxCoupledClients = 1; // One btproxy and one windspc

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET PCClientSocket, BTProxyClientSocket;
	SOCKET TempClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvid[DEFAULT_MSGLEN] = "";
	char msgfrompc[DEFAULT_MSGLEN] = "";
    int recvidlen = DEFAULT_IDLEN;
	int recvmsglen = DEFAULT_MSGLEN;
    
	// Initialize ClientSockets
	PCClientSocket = INVALID_SOCKET;
	BTProxyClientSocket = INVALID_SOCKET;
	
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
	printf("WSA Inited\n");

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
	printf("Finished bind\n");
    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
	printf("Listening on socket %d\n", ListenSocket);
	
	// Create sockets for pc and btproxy
	for (i = 0; i < 2*MaxCoupledClients; i++) {
		// Since we don't know which of the btproxy or the pc will connect first,
		// we keep in TempClientSocket the new client socket, and after we receive the id ("windspc" or "btproxy")
		// we will save at PCClientSocket the pc socket and at BTProxyClientSocket the btproxy socket.
		
		// Accept a client socket
		printf("Before accept with i = %d\n", i);
		TempClientSocket = accept(ListenSocket, NULL, NULL);
		if (TempClientSocket == INVALID_SOCKET) {
			printf("accept failed with error: %d, i = %d\n", WSAGetLastError(), i);
			closesocket(TempClientSocket);
			WSACleanup();
			return 1;
		}
		printf("Accepted connection on TempClientSocket %d\n", TempClientSocket);
		printf("Waiting to receive the id...\n");
		
		iResult = recv(TempClientSocket, recvid, recvidlen, 0);
        if (iResult > 0) {
            printf("Bytes received: %d\n", iResult);
			
			if (strcmp(recvid, "windspc") == 0) {
				printf("Recieved pc\n");
				PCClientSocket = TempClientSocket;
				printf("Waiting for message from pc...\n");
				iResult = recv(PCClientSocket, msgfrompc, recvmsglen, 0);
				if (iResult > 0) {
					printf("Bytes received from pc: %d\n", iResult);
					printf("Received from pc: %s\n", msgfrompc);
				} else {
					printf("recv from pc failed with error: %d\n", WSAGetLastError());
					closesocket(PCClientSocket);
					closesocket(BTProxyClientSocket);
					WSACleanup();
					return 1;
				}
			} else if (strcmp(recvid,"btproxy") == 0) {
				BTProxyClientSocket = TempClientSocket;
				printf("Recieved btproxy\n");
			} else {
				printf("received %s which is not a correct id\n", recvid);
			}
        }
        else if (iResult == 0) {
            printf("Connection closing...\n");
        } else  {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(TempClientSocket);
            WSACleanup();
            return 1;
        }
	}
	
    
	// send data from pc socket to btproxy socket
	printf("Sending message %s to btproxy...\n", msgfrompc);
	iSendResult = send( BTProxyClientSocket, msgfrompc, strlen(msgfrompc), 0 );
	if (iSendResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(PCClientSocket);
		closesocket(BTProxyClientSocket);
		WSACleanup();
		return 1;
	}
	printf("Bytes sent to btproxy: %d\n", iSendResult);
    
    // No longer need server socket
    closesocket(ListenSocket);

    // shutdown the connection since we're done
	iResult = shutdown(BTProxyClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(BTProxyClientSocket);
	}
	iResult = shutdown(PCClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(PCClientSocket);
		WSACleanup();
		return 1;
	}
	
    // cleanup
	closesocket(PCClientSocket);
	closesocket(BTProxyClientSocket);
    WSACleanup();

    return 0;
}
