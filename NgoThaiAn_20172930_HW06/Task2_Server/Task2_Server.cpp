#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <conio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <process.h>

#define BUFF_SIZE 2048
#pragma comment(lib,"Ws2_32.lib")


typedef struct {
	SOCKET connSock;
	char addr[BUFF_SIZE];
}node;

node sock[FD_SETSIZE];


node createNode(SOCKET connSock, char user[])
{
	node p;
	p.connSock = connSock;
	strcpy(p.addr, user);
	return p;
}
int receiveData(SOCKET, char*, int, int);
int sendData(SOCKET, char*, int, int);
void name(SOCKET, char*);
void iP(SOCKET, char*);
int main(int argc, char* argv[])
{
	int SERVER_PORT = atoi(argv[1]);
	//Step 1: Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported\n");

	//Step 2: Construct socket	
	SOCKET listenSock;
	//unsigned long ul = 1;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//ioctlsocket(listenSock, FIONBIO, (unsigned long*)&ul);

	//Step 3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error! Cannot bind this address.");
		_getch();
		return 0;
	}

	//Step 4: Listen request from client
	if (listen(listenSock, 10)) {
		printf("Error! Cannot listen.");
		_getch();
		return 0;
	}

	printf("Server started!\n");

	//Step 5: Communicate with client
	SOCKET client[FD_SETSIZE], connSock;
	fd_set readfds;
	sockaddr_in clientAddr;
	int ret, nEvents, clientAddrLen;
	char rcvBuff[BUFF_SIZE];
	for (int i = 0; i < FD_SETSIZE; i++)
		client[i] = 0;
	FD_ZERO(&readfds);

	while (1)
	{

		FD_SET(listenSock, &readfds);
		for (int i = 0; i < FD_SETSIZE; i++)
			if (client[i] > 0)
				FD_SET(client[i], &readfds);

		nEvents = select(0, &readfds, 0, 0, 0);
		if (nEvents < 0) {
			printf("\nError! Cannot poll sockets: %d", WSAGetLastError());
			break;
		}
		if (FD_ISSET(listenSock, &readfds))
		{
			clientAddrLen = sizeof(clientAddr);
			connSock = accept(listenSock, (sockaddr*)&clientAddr, &clientAddrLen);
			if (connSock != INVALID_SOCKET)
			{
				int i;
				for (i = 0; i < FD_SETSIZE; i++)
					if (client[i] <= 0)
					{
						//printf("new connection\n");
						client[i] = connSock;
						//sock[i] = createNode(client[i], "\0");
						break;
					}
				if (i == FD_SETSIZE) {
					printf("\nToo many clients.");
				}
				if (--nEvents <= 0) continue;
			}

		}
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (client[i] <= 0) continue;
			if (FD_ISSET(client[i], &readfds)) {
				ret = receiveData(client[i], rcvBuff, BUFF_SIZE, 0);
				if (ret <= 0)
				{
					FD_CLR(client[i], &readfds);
					closesocket(client[i]);
					client[i] = 0;
				}
				else if (ret > 0)
				{
					unsigned long ulAddr = inet_addr(rcvBuff);
					//check the input by inet_addr()
					if (ulAddr == INADDR_NONE || ulAddr == INADDR_ANY) name(client[i], rcvBuff);
					else iP(client[i], rcvBuff);
				}
			}
			if (--nEvents <= 0) continue;
		}
	}
	_getch();
	return 0;
}
int receiveData(SOCKET connSock, char* buff, int size, int flag)
{
	int n = recv(connSock, buff, size, flag);
	if (n <= 0)
	{
		printf("Error receive data.\n");
		return -1;
	}
	buff[n] = 0;
	return n;
}
int sendData(SOCKET connSock, char* buff, int size, int flag)
{
	int n = send(connSock, buff, size, flag);
	if (n <= 0)
	{
		printf("Error send data.\n");
		return -1;
	}
	return n;
}
void name(SOCKET server, char* buff)
{
	int dwRetval;

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

	dwRetval = getaddrinfo(buff, "http", &hints, &result);
	if (dwRetval != 0) {
		sendData(server, "f", 1, 0);
		
	}
	else
	{
		int k = 0;
		char message[BUFF_SIZE];
		ZeroMemory(message, BUFF_SIZE);
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
			struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
			sprintf(message, "%s%c%s", message,(char)'A' + k, inet_ntoa(sockaddr_ipv4->sin_addr));
			puts(message);
			k++;
		}
		char sendMes[BUFF_SIZE];
		sprintf(sendMes, "%c%s", 'i', message);
		puts(sendMes);
		sendData(server, sendMes, strlen(sendMes), 0);
		//message={'i','A','ipAddr1','B','ipAddr2',...}
	}
	freeaddrinfo(result);
	return;
}
void iP(SOCKET server, char *buff)
{
	struct hostent *remoteHost;
	char *host_addr = buff;
	struct in_addr addr = { 0 };
	addr.s_addr = inet_addr(host_addr);
	char message[BUFF_SIZE];
	ZeroMemory(message, BUFF_SIZE);
	remoteHost = gethostbyaddr((char *)&addr, 4, AF_INET);
	if (remoteHost == NULL) sendData(server, "f", 1, 0);
	else {
		sprintf(message, "%c%s", 'A', remoteHost->h_name);
		char **pAlias;
		int k = 0;
		for (pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++) {
			sprintf(message, "%s%c%s", message, 'B' + k, *pAlias);
			k++;
		}
		char sendMes[BUFF_SIZE];
		sprintf(sendMes, "%c%s", 'h', message);
		//message={'h','A','hostname','B','alias',...}
		puts(sendMes);
		sendData(server, sendMes, strlen(sendMes), 0);
	}
}
