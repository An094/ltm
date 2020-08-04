#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define BUFF_SIZE 2048
#pragma comment(lib,"Ws2_32.lib")

int Send(SOCKET, char*, int, int);
int Receive(SOCKET, char*, int, int);
void display(char*, char*);
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

	char buff[BUFF_SIZE];
	int ret;
	//Step 5: Communicate with server
	while (1) {
		printf("\nSend to server: ");
		gets_s(buff, BUFF_SIZE);
		if (buff == NULL)
			break;//break loop if buff is NULL(condition end loop)
		ret = Send(client, buff, strlen(buff), 0);
		if (ret == SOCKET_ERROR)
			printf("Error: %d", WSAGetLastError());


		ret = Receive(client,buff,BUFF_SIZE,0);
		if (ret == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAETIMEDOUT)
				printf("Time-out!");
			else printf("Error! Cannot receive message");
		}
		else if (strlen(buff) > 0)
		{
			if (buff[0] == 'f')//Prefix is f.
			{
				printf("Not found information");
			}
			else
			{
				if (buff[0] == 'i') display(buff, "IP");
				else display(buff, "name");//prefix is 'h'
			}
		}
	}
	//Step 6: Close socket
	closesocket(client);

	//Step 7: Terminate Winsock
	WSACleanup();

	_getch();
	return 0;
}

int Send(SOCKET client, char* buff, int size, int flag)
{
	int n;
	n = send(client, buff, size, flag);
	if (n == SOCKET_ERROR)
	{
		printf("Error! Code: %d\n", WSAGetLastError());
	}
	return n;
}
int Receive(SOCKET client, char* buff, int size, int flag)
{
	int n;
	n = recv(client, buff, size, flag);
	if (n == SOCKET_ERROR)
	{
		printf("Error! Code: %d\n", WSAGetLastError());
	}
	buff[n] = 0;
	return n;
}
void display(char *buff, char *code)
{
	int bufflen = strlen(buff);
	for (int i = 1; i < bufflen; i++)
	{

		//message ={'h or i','A','IP or name official','B','Alias','C',...}.
		if (buff[i] == 'A') printf("\nOfficial %s: ", code);//if kind is IP print Official IP.
		else if (buff[i] == 'B') printf("\nAlias: ");
		else if (buff[i] > 'B'&&buff[i] < 'Z') printf("\n");
		else printf("%c", buff[i]);
	}
}