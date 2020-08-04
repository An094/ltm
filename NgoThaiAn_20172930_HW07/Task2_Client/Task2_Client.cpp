#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <conio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <process.h>
#pragma comment(lib,"Ws2_32.lib")
#define BUFF_SIZE 2048
#define DATA_SIZE 65
char link[BUFF_SIZE];
void menu(SOCKET);
void process(SOCKET, int);
int Send(SOCKET, char*, int, int);
int Receive(SOCKET, char*, int, int);
void processMessage(char*, SOCKET);
void receiveData(char*, SOCKET);
void sendData(char*, SOCKET);
void recvMessage(SOCKET);


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
	while (true)
	{
		menu(client);
	}
	//Step 6: Close socket
	closesocket(client);

	//Step 7: Terminate Winsock
	WSACleanup();

	_getch();
	return 0;
}
void menu(SOCKET connSock)
{
	system("cls");
	printf("1. Encryption\n");
	printf("2. Decryption\n");
	printf("Choose: \n");
	int choose;
	scanf("%d", &choose);
	process(connSock, choose);

}
void process(SOCKET connSock, int choose)
{
	int key;
	printf("key: ");
	scanf("%d", &key);
	char firstMessage[BUFF_SIZE];
	sprintf(firstMessage, "%d00%d", choose - 1, key);
	Send(connSock, firstMessage, strlen(firstMessage), 0);
	fflush(stdin);
	printf("link file: ");
	scanf("%s", link);

	if (strlen(link) == 0) return;
	for (int i = 0; i < strlen(link); i++)
	{
		if (link[i] == 92) link[i] = 47;
	}

	sendData(link, connSock);
	recvMessage(connSock);

}
void sendData(char link[], SOCKET connSock)
{
	FILE *fp = fopen(link, "r");
	int i = 0;
	char result[DATA_SIZE];
	char sendMess[BUFF_SIZE];
	while (1)
	{

		result[i++] = fgetc(fp);
		if (feof(fp))
		{
			result[i] = 0;
			sprintf(sendMess, "2%d%s", strlen(result), result);
			Send(connSock, sendMess, strlen(sendMess), 0);
			break;
		}
		else if (i == DATA_SIZE - 1)
		{
			result[i] = 0;
			sprintf(sendMess, "2%d%s", strlen(result), result);
			Send(connSock, sendMess, strlen(sendMess), 0);
			char check[BUFF_SIZE];
			Receive(connSock, check, BUFF_SIZE, 0);
			if (strcmp(check, "OK") != 0) printf("Error");
			ZeroMemory(result, DATA_SIZE);
			ZeroMemory(sendMess, BUFF_SIZE);
			i = 0;
		}
	}
	char check[BUFF_SIZE];
	Receive(connSock, check, BUFF_SIZE, 0);
	if (strcmp(check, "OK") != 0) printf("Error");
	Send(connSock, "200", 3, 0);
	fclose(fp);
	return;
}
void recvMessage(SOCKET connSock)
{
	char message[BUFF_SIZE];
	int ret;
	do {
		ret = Receive(connSock, message, BUFF_SIZE, 0);
		if (ret < 0)
		{
			printf("Error!");
			return;
		}
		Send(connSock, "OK", 2, 0);
		processMessage(message, connSock);
	} while (ret > 0);
	return;
}
int Send(SOCKET connSock, char* buff, int size, int flag)
{
	int ret = send(connSock, buff, size, flag);
	if (ret < 0) printf("Error! code: %d", WSAGetLastError());
	return ret;
}
int Receive(SOCKET connSock, char* buff, int size, int flag)
{
	int ret = recv(connSock, buff, size, flag);
	if (ret < 0) printf("Error! code: %d", WSAGetLastError());
	else buff[ret] = 0;
	return ret;
}
void processMessage(char message[], SOCKET connSock)
{
	if (message[0] == '2') receiveData(message, connSock);
	else if (message[0] == '3') printf("Error receive data from server!\n");
	return;
}

void receiveData(char message[], SOCKET connSock)
{
	if (message[1] != '0' || message[2] != '0')
	{
		int lenMess = (message[1] - 48) * 10 + message[2] - 48;
		char linkFile[BUFF_SIZE];
		sprintf(linkFile, "%s.enc", link);
		
		FILE *fp = fopen(linkFile, "a");
		for (int i = 0; i < lenMess; i++)
		{
			fputc(message[i + 3], fp);
		}
		fclose(fp);
		return;
	}
	else
	{
		printf("Transfer data is successfull\n");
		return;
	}

}
