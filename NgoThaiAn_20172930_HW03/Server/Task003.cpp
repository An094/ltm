#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS
#include <conio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>

#define BUFF_SIZE 2048

#pragma comment(lib,"Ws2_32.lib")

int reciveMessage(SOCKET, char*);
int sendMessage(SOCKET, char*, int);
int proc(char*);

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
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

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
	sockaddr_in clientAddr;
	char buff[BUFF_SIZE];
	int  clientAddrLen = sizeof(clientAddr);


	SOCKET connSock;

	while (1)
	{//accept request
		connSock = accept(listenSock, (sockaddr *)& clientAddr, &clientAddrLen);
		while (1) {
			//receive message from client
			int result = reciveMessage(connSock, buff);
			if (result == SOCKET_ERROR) break;
		} //end accepting
		closesocket(connSock);
	}
	//Step 5: Close socket
	closesocket(listenSock);

	//Step 6: Terminate Winsock
	WSACleanup();

	return 0;
}
int reciveMessage(SOCKET connSock, char*buff)
{
	int ret;
	ret = recv(connSock, buff, 4, 0);//recive length of message from  client
	if (ret == SOCKET_ERROR) return SOCKET_ERROR;
	int recvlen = atoi(buff);
	int index = 0;
	while (recvlen > 0)//repeat until enough data is received
	{
		ret = recv(connSock, buff + index, BUFF_SIZE, 0);
		if (ret == SOCKET_ERROR) {
			printf("Error : %d\n", WSAGetLastError());
			return SOCKET_ERROR;
		}
		index += ret;//next position
		recvlen -= ret;
	}
	if (strlen(buff) > 0) 
	{
		buff[index] = 0;
		puts(buff);
		int count = proc(buff);
		//Echo to client
		return sendMessage(connSock, buff, count);
	}

}
int sendMessage(SOCKET connSock, char* buff, int count)
{
	if (count == -1) send(connSock, "f", 1, 0);//If the message has a number, send a message that begins with "f"
	else
	{
		char temp[10];
		sprintf_s(temp, "%d", count);//convert int ->char*
		int ret = send(connSock, temp, strlen(temp), 0);
		if (ret == SOCKET_ERROR)
		{
			printf("Error! Code: %d", WSAGetLastError());
			return SOCKET_ERROR;
		}
	}
}
int proc(char* buff)
{
	int bufflen = strlen(buff);
	int count = 1;
	for (int i = 0; i < bufflen; i++)
	{
		if (buff[i] >= '0'&&buff[i] <= '9') return -1;//if the message has number return -1;
		else if (buff[i] == ' ') count++;//count = amount of space+1;
	}
	return count;
}

