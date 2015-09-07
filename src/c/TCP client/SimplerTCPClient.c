#include<stdio.h>
#include<winsock2.h>
 
#pragma comment(lib,"ws2_32.lib") //Winsock Library
 
int main(int argc , char *argv[])
{
    WSADATA wsa;
    SOCKET s;
    struct sockaddr_in server;
    char *message;
	
 	// =========================================
    // Initialize Winsock
	// ========================================= 
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        printf("Failed. Error Code : %d",WSAGetLastError());
        return 1;
    }
     
    printf("Initialised.\n");
     
	// =========================================
	// Create a SOCKET for connecting to server
	// =========================================
    if((s = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d" , WSAGetLastError());
    }
 
    printf("Socket created.\n");
     
	// =========================================
	// Define the server address
	// =========================================     
    server.sin_addr.s_addr = inet_addr("132.68.53.90");
    server.sin_family = AF_INET;
    server.sin_port = htons( 4020 );
 
	// =========================================
	// Connect to server
	// =========================================
    if (connect(s , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("connect error");
        return 1;
    }
     
    puts("Connected");
     
	// =========================================
	// Send an initial buffer
	// =========================================
    message = "Message from TCP Client";
    if( send(s , message , strlen(message) , 0) < 0)
    {
        puts("Send failed");
        return 1;
    }
    puts("Data Send\n");
	
	// =========================================
    // cleanup
	// =========================================
	printf("Cleaning up\n");
    closesocket(s);
    WSACleanup();	
	
 
    return 0;
}