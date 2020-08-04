// server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<conio.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<string.h>


#define BUFF_SIZE 2048
#define MAX 64
#pragma comment(lib,"Ws2_32.lib")


void name(SOCKET,char*,sockaddr_in);
void iP(SOCKET,char*,sockaddr_in);
char* mess(char*, char);
char* link(char*, char*);


int main(int argc, char* argv[])
{
	//Step1: Initiate Winsock
	int SERVER_PORT = atoi(argv[1]);//convert *char to int-->SERVER_PORT
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is a not supported\n");
	//Step2: Construct socket
	SOCKET server;
	server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//Step3: Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	if (bind(server, (sockaddr*)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error! Can not bind this address");
		_getch();
		return 0;
	}
	printf("Server started!\n");

	//Step 4: Communicate with client
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE];
	int ret, clientAddrLen = sizeof(clientAddr);

	while (1)
	{
		//ZeroMemory(buff, BUFF_SIZE);
		ret = recvfrom(server, buff, BUFF_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrLen);
		if (ret == SOCKET_ERROR)
			printf("Error: %d", WSAGetLastError());
		else if (strlen(buff) > 0)
		{
			buff[ret] = 0;
			//message
			puts(buff);
			unsigned long ulAddr = inet_addr(buff);
			//check the input by inet_addr()
			if (ulAddr == INADDR_NONE || ulAddr == INADDR_ANY) name(server,buff,clientAddr);
			else iP(server,buff,clientAddr);
		}
	}
	closesocket(server);
	WSACleanup();
	return 0;
}


void name(SOCKET server,char* buff,sockaddr_in clientAddr)
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

	dwRetval = getaddrinfo(buff, "http", &hints, &result);
	if (dwRetval != 0) {
		char *m = mess(buff, 'f');
		sendto(server, m, strlen(m), 0, (sockaddr*)&clientAddr,sizeof(clientAddr) );
	}
	else
	{
		int k = 0;
		char *message = "i";
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
			struct sockaddr_in *sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
			char *m = mess(inet_ntoa(sockaddr_ipv4->sin_addr), 'A' + k);//Insert prefix before message
			message = link(message, m);//link two array
			k++;
		}
		sendto(server, message, strlen(message), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
		//message={'i','A','ipAddr1','B','ipAddr2',...}
	}
	freeaddrinfo(result);
}
void iP(SOCKET server, char *buff, sockaddr_in clientAddr)
{
	struct hostent *remoteHost;
	char *host_addr = buff;
	struct in_addr addr = { 0 };
	addr.s_addr = inet_addr(host_addr);
	char *message = "h";

	remoteHost = gethostbyaddr((char *)&addr, 4, AF_INET);
	if (remoteHost == NULL) sendto(server, "f", 1, 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
	else {
		char *m = mess(remoteHost->h_name, 'A');
		message = link(message, m);
		char **pAlias;
		int k = 0;
		for (pAlias = remoteHost->h_aliases; *pAlias != 0; pAlias++) {
			char *temp = mess(*pAlias, 'B' + k);
			message = link(message, temp);
			k++;
		}
		//message={'h','A','hostname','B','alias',...}
		sendto(server, message, strlen(message), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
	}
}
char *mess(char* buff, char insert)//insert in ahead array
{
	int bufflen = strlen(buff);
	char *buff1 =(char*)malloc(bufflen+1);
	for (int i = 1; i <= bufflen; i++)
	{
		buff1[i] = buff[i - 1];
	}
	buff1[0] = insert;
	buff1[bufflen + 1] = '\0';
	return buff1;
}
char *link(char *mess1, char*mess2)//link two array
{
	int mess1len = strlen(mess1);
	int mess2len = strlen(mess2);
	int messlen = mess1len + mess2len;
	char *message = (char*)malloc(messlen + 1);
	for (int i = 0; i<mess1len; i++)
	{
		message[i] = mess1[i];
	}
	for (int j = 0; j<mess2len; j++)
	{
		message[mess1len + j] = mess2[j];
	}
	message[messlen] = '\0';
	return message;
}