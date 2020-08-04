#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define BUFF_SIZE 2048

#pragma comment(lib,"Ws2_32.lib")

void sendMessage(SOCKET, char*, int);
void reciveMessage(SOCKET, char*);
int main(int argc, char* argv[])
{
	int SERVER_PORT = atoi(argv[2]);
	char* SERVER_ADDR = argv[1];
	//Step 1: Inittiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported\n");

	//Step 2: Construct socket	
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//(optional) Set time-out for receiving
	int tv = 10000; //Time-out interval: 10000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));

	//Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	//Step 4: Request to connect server
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error! Cannot connect server. %d", WSAGetLastError());
		_getch();
		return 0;
	}
	printf("Connected server!\n");

	//Step 5: Communicate with server
	char buff[BUFF_SIZE];
	int ret;
	while (1) {
		//Send message
		ZeroMemory(buff, BUFF_SIZE);
		printf("\nSend to server: ");
		gets_s(buff, BUFF_SIZE);
		//puts(buff);
		int bufflen = strlen(buff);
		//printf("%d", bufflen);
		if (bufflen == 0) break;
		sendMessage(client, buff, bufflen);
		//send(client, buff, strlen(buff), 0);
		//Receive echo message
		reciveMessage(client, buff);
	}
	//Step 6: Close socket
	closesocket(client);

	//Step 7: Terminate Winsock
	WSACleanup();

	_getch();
	return 0;
}
void sendMessage(SOCKET client, char *buff, int bufflen)
{
	int ret, index = 0;
	char temp[4];//message array 
	sprintf_s(temp, "%d", bufflen);
	
	ret = send(client, temp, 4, 0);//send length of message

	while (bufflen > 0)//repeat until enough data 
	{
		ret = send(client, buff + index, bufflen, 0);
		if (ret == SOCKET_ERROR)
		{
			printf("Error! Code: %d", WSAGetLastError());
			return;
		}
		else
		{
			index += ret;
			bufflen -= ret;
		}
	}
}
void reciveMessage(SOCKET client, char* buff)
{
	int ret;
	ret = recv(client, buff, BUFF_SIZE, 0);
	if (ret == SOCKET_ERROR)//Error
	{
		if (WSAGetLastError() == WSAETIMEDOUT)
			printf("Time-out!");
		else printf("Error! Cannot receive message.");
	}
	else
	{
		buff[ret] = 0;//End of message
		if (buff[0] == 'f') printf("Error: String contains number. ");//If prefix is 'f', message send have number
		else
		{
			printf("Number of word: ");
			for (int i = 0; i < ret; i++) printf("%c", buff[i]);
		}
	}
}