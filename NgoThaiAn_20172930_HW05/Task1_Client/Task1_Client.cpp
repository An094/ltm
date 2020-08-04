#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <WinSock2.h>
#include <WS2tcpip.h>

#define BUFF_SIZE 2048
#define MAX_ARRAY 50
#pragma comment(lib,"Ws2_32.lib")
void menu(SOCKET);
void login(SOCKET);
void logout(SOCKET);
int Send(SOCKET, char*, int, int);
int Receive(SOCKET, char*, int, int);
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
	while (1) {
		menu(client);
	}
	//Step 6: Close socket
	closesocket(client);

	//Step 7: Terminate Winsock
	WSACleanup();

	_getch();
	return 0;
}
void login(SOCKET client)
{
	system("cls");
	char user[BUFF_SIZE];
	char pass[BUFF_SIZE];
	printf("<username> <password>:\n");
	gets_s(user, BUFF_SIZE);
	gets_s(pass, BUFF_SIZE);
	char info[BUFF_SIZE];
	strcpy(info, "LOGIN");
	strcat(info, user);
	strcat(info, " ");
	strcat(info, pass);
	//puts(info);
	//_getch();
	int leninfo = strlen(info);
	Send(client, info, leninfo, 0);
	char recvBuff[BUFF_SIZE];
	int ret = Receive(client, recvBuff, BUFF_SIZE, 0);
	recvBuff[ret] = 0;
	puts(recvBuff);
	printf("Press to continue");
	_getch();
}
void logout(SOCKET client)
{
	system("cls");
	int size = 6;
	Send(client, "LOGOUT", size, 0);
	char recvBuff[BUFF_SIZE];
	int ret = Receive(client, recvBuff, BUFF_SIZE, 0);
	recvBuff[ret] = 0;
	puts(recvBuff);
	printf("Press to continue");
	_getch();
}
void exitSession(SOCKET client)
{
	system("cls");
	int size = 6;
	Send(client, "EXIT  ", size, 0);
	char recvBuff[BUFF_SIZE];
	int ret = Receive(client, recvBuff, BUFF_SIZE, 0);
	recvBuff[ret] = 0;
	if (strcmp(recvBuff, "OK") == 0) exit(1);
	else
	{
		puts(recvBuff);
		_getch();
		printf("Press to continue");
	}
}

void menu(SOCKET client)
{
	int choise;
	system("cls");
	printf("==========MENU==========\n");
	printf("1. Login\n");
	printf("2. Logout\n");
	printf("3. Exit\n");
	printf("========================\n");
	printf("Your choise?\n");
	scanf("%d", &choise);
	switch (choise)
	{
	case 1:
	{
		login(client);
		break;
	}
	case 2:
	{
		logout(client);
		break;
	}
	case 3:
	{
		exitSession(client);
	}
	default:
		break;
	}
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
	return n;
}