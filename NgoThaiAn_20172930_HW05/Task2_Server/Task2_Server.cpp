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
#define MAX_NODE 10
unsigned __stdcall mainProc(void*);
int Send(SOCKET,char*,int, int);
int Receive(SOCKET,char*,int,int);
void processMessage(char*,int);
char encryption(char,int);
char decryption(char,int);
void receiveData(char*,int);
void processFile(int);
void sendData(SOCKET);
void errorMessage(SOCKET);

const char* fileNameTemp = "Temp.txt";
const char* fileNameRes = "Result.txt";

typedef struct {
	SOCKET connSock;
	int mode;
	int key;
}node;
node sock[MAX_NODE];
node createNode(SOCKET connSock, int mode, int key)
{
	node p;
	p.connSock = connSock;
	p.mode = mode;
	p.key = key;
	return p;
}
int num = 0;

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
	//char buff[BUFF_SIZE];
	int  clientAddrLen = sizeof(clientAddr);

	SOCKET connSock;
	while (true)
	{//accept request
		connSock = accept(listenSock, (sockaddr *)& clientAddr, &clientAddrLen);
		sock[num++] = createNode(connSock, 0, 0);
		_beginthreadex(0, 0, mainProc, (void*)&num, 0, 0);
	}
	//Step 5: Close socket
	closesocket(listenSock);

	//Step 6: Terminate Winsock
	WSACleanup();

	return 0;
}
unsigned __stdcall mainProc(void* param)
{
	int number_sock = *(int*)param - 1;
	SOCKET connSock = sock[number_sock].connSock;
	char message[BUFF_SIZE];
	while (true)
	{
		int ret = Receive(connSock, message, BUFF_SIZE, 0);
		if (ret < 0) break;
		Send(connSock, "OK", 2, 0);
		//puts(message);
		//printf("|||");
		processMessage(message, number_sock);
		
	}
	shutdown(connSock, SD_SEND);
	closesocket(sock[number_sock].connSock);
	return 0;
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
void processMessage(char message[],int number_sock)
{
	if (message[0] == '0' || message[0] == '1')
	{
		//int lenKey = message[2] - 48;
		int key = atoi(message + 3);
		sock[number_sock].key = key;
		sock[number_sock].mode = message[0] - 48;
	}
	else if (message[0] == '2')
	{
		//FILE *fp = fopen(fileNameTemp, "a");
		receiveData(message,number_sock);
	}
}
char encryption(char message,int key)
{
	return (message + key) % 256;
}
char decryption(char message,int key)
{
	return (message - key) % 256;
}
void receiveData(char message[],int number_sock)
{
	if (message[1] == '0'&&message[2] == '0')
	{
		processFile(number_sock);
		sendData(sock[number_sock].connSock);
		char cmd[BUFF_SIZE];
		sprintf(cmd, "del %s", fileNameTemp);
		system(cmd);
		sprintf(cmd, "del %s", fileNameRes);
		system(cmd);

	}
	else
	{
		int lenMess = (message[1] - 48) * 10 + message[2] - 48;
		
		FILE *fp = fopen(fileNameTemp, "a");
		for (int i = 0; i < lenMess; i++)
		{
			fputc(message[i + 3], fp);
			//fgetc(fp);
		}
		fclose(fp);
	}
}
void processFile(int number_sock)
{
	FILE *fp = fopen(fileNameTemp, "r");
	FILE *fpres = fopen(fileNameRes, "w");
	int code = sock[number_sock].mode;
	int key = sock[number_sock].key;
	if (code == 0)
	{
		while (!feof(fp))
		{
			char temp = fgetc(fp);
			fputc(encryption(temp,key), fpres);
		}
	}
	else
	{
		while (!feof(fp))
		{
			char temp = fgetc(fp);
			fputc(decryption(temp,key), fpres);
		}
	}
	fclose(fp);
	fclose(fpres);
}
void sendData(SOCKET connSock)
{
	FILE *fpres = fopen(fileNameRes, "r");
	int i = 0;
	char result[DATA_SIZE];
	char sendMess[BUFF_SIZE];
	while (1)
	{

		result[i++] = fgetc(fpres);
		if (feof(fpres))
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
			if (strcmp(check, "OK") != 0) break;
			ZeroMemory(result, DATA_SIZE);
			ZeroMemory(sendMess, BUFF_SIZE);
			i = 0;
		}
	}
	char check[BUFF_SIZE];
	Receive(connSock, check, BUFF_SIZE, 0);
	if (strcmp(check, "OK") != 0) printf("Error");
	Send(connSock, "200", 3, 0);
	fclose(fpres);
}
void errorMessage(SOCKET connSock)
{
	Send(connSock, "3", 1, 0);
}
