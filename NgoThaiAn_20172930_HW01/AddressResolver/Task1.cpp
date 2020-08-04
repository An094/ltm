// AddressResolver.cpp : Defines the entry point for the console application.

#include "stdafx.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#undef UNICODE

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
 

// link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

void name(char*);
void iP(char*);

int main(int argc, char* argv[])
{
	char *INPUT = argv[1];
	
	WSADATA wsaData;
	

	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);   // Initialize Winsock
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		printf("Error code: %d", WSAGetLastError());
		return 1;
	}
	unsigned long ulAddr = inet_addr(INPUT);
	if (ulAddr == INADDR_NONE || ulAddr == INADDR_ANY) name(INPUT);
	else iP(INPUT);
	
	WSACleanup();


	return 0;
}
void name(char* INPUT)
{
	

	INT dwRetval;

	struct addrinfo *result = NULL;
	struct addrinfo *ptr = NULL;
	struct addrinfo hints;

	// Setup the hints address info structure
	// which is passed to the getaddrinfo() function
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;


	//--------------------------------
	// Call getaddrinfo(). If the call succeeds,
	// the result variable will hold a linked list
	// of addrinfo structures containing response
	// information
	dwRetval = getaddrinfo(INPUT, "http", &hints, &result);
	if (dwRetval != 0) {
		printf("Not found information");
	}else
	{
		printf("List of IPs:\n");
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
			struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
			printf("%s\n", inet_ntoa(sockaddr_ipv4->sin_addr));
		}
	}
	freeaddrinfo(result);

}
void iP(char* INPUT)
{
	struct sockaddr_in saGNI;
	char hostname[NI_MAXHOST];
	char servInfo[NI_MAXSERV];

	//Set up sockaddr_in structure which is passed to the getnameinfo function
	saGNI.sin_family = AF_INET;
	saGNI.sin_addr.s_addr = inet_addr(INPUT);

	//-----------------------------------------
	// Call getnameinfo
	int dwRetval;
	dwRetval = getnameinfo((struct sockaddr *) &saGNI,
		sizeof(struct sockaddr),
		hostname,
		NI_MAXHOST, servInfo, NI_MAXSERV, NI_NAMEREQD);

	if (dwRetval != 0) {
		printf("Not found information");
		return ;
	}
	else
		printf("List of name: \n");
		puts(hostname);
}

