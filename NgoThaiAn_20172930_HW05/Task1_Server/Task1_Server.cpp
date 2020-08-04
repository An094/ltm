#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <conio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>
#include <process.h>

#define BUFF_SIZE 2048
#define MAX_ARRAY 50
#define MAX_HEADER 7
#define MAX 10
#pragma comment(lib,"Ws2_32.lib")
typedef struct account {
	char username[BUFF_SIZE];
	char password[BUFF_SIZE];
	char status;
	int countFail;
}acc;

typedef struct {
	SOCKET connSock;
	char current_user[BUFF_SIZE];
}node;

node sock[MAX];
int num = 0;
const char* fileName = "account.txt";

node createNode(SOCKET connSock, char user[])
{
	node p;
	p.connSock = connSock;
	strcpy(p.current_user, user);
	return p;
}
acc* getAccountFromFile();
int receiveMess(SOCKET,char*,int,int);
int sendMess(SOCKET,char*,int,int);
void login(char*, char*, int,acc*);
void logout(int);
void exitSession(int);
void updateAccList(acc*, int);
int echoMess(SOCKET, int);
unsigned __stdcall Proccess(void*);

//char current_account[BUFF_SIZE];
int amountOFAccountInFile;

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
		//strcpy(current_account, "\0");
		sock[num++] = createNode(connSock, "\0");
		_beginthreadex(0, 0, Proccess, (void*)&num, 0, 0);
	}
	//Step 5: Close socket
	closesocket(listenSock);

	//Step 6: Terminate Winsock
	WSACleanup();

	return 0;
}
acc* getAccountFromFile()
{
	FILE *fp = fopen(fileName, "r");
	acc* accListFile = (acc*)malloc(MAX_ARRAY * sizeof(acc));
	char accountFile[BUFF_SIZE];
	char userFile[BUFF_SIZE];
	char passFile[BUFF_SIZE];
	char statusFile;
	int len = 0;
	int countLine = 0;
	while (!feof(fp))
	{
		fgets(accountFile, sizeof(accountFile), fp);
		len = strlen(accountFile);
		if (accountFile[len - 1] == '\n')
			accountFile[len - 1] = '\0';
		int position = 0, j = 0, k = 0;
		for (int i = 0; i < len; i++)
		{
			if (accountFile[i] == ' ') position++;

			else if (position == 0)
			{
				userFile[j++] = accountFile[i];
			}
			else if (position == 1)
			{
				passFile[k++] = accountFile[i];
			}
			else if (position == 2)
			{
				statusFile = accountFile[i];
				break;
			}
		}
		userFile[j] = 0;
		passFile[k] = 0;

		strcpy(accListFile[countLine].username, userFile);
		strcpy(accListFile[countLine].password, passFile);
		accListFile[countLine].status = statusFile;

		countLine++;

	}
	amountOFAccountInFile = countLine;
	fclose(fp);
	accListFile = (acc*)realloc(accListFile, countLine * sizeof(acc));
	return accListFile;
}
void login(char* user, char* pass, int number_socket,acc* accList)
{
	SOCKET connSock = sock[number_socket].connSock;

	if (strcmp(sock[number_socket].current_user,"\0")!=0)
	{
		echoMess(connSock, 14);
		return;
	}
	for (int i = 0; i <= MAX; i++)
	{
		if (strcmp(sock[i].current_user, user) == 0)
		{
			echoMess(connSock, 17);
			return;
		}
	}
	//acc* accList = getAccountFromFile();
	int i;
	for (i = 0; i < amountOFAccountInFile; i++)
	{
		if (strcmp(accList[i].username, user) == 0)
		{
			if ((strcmp(accList[i].password, pass) == 0) && (accList[i].status == '0'))
			{
				int ret = echoMess(connSock, 11);
				printf("Login Successful\n");
				strcpy(sock[number_socket].current_user, user);
				accList[i].countFail = 0;
				return;
			}
			else if (accList[i].status == '1')
			{
				echoMess(connSock, 15);
				printf("Login Failed.\n");
				return;
			}
			else if (strcmp(accList[i].password, pass) != 0)
			{
				accList[i].countFail++;
				printf("Login Failed.\n");
				if (accList[i].countFail == 3)
				{
					printf("Account is block.\n");
					echoMess(connSock, 16);
					accList[i].status = '1';
					updateAccList(accList, amountOFAccountInFile);
				}
				else echoMess(connSock, 13);
			}
			break;
		}

	}
	if (i == amountOFAccountInFile) echoMess(connSock, 12);
	return;
}
void logout(int number_socket)
{
	SOCKET connSock = sock[number_socket].connSock;
	//char current_account[BUFF_SIZE];
	//strcpy(current_account, sockNode.current_user);
	if (strcmp(sock[number_socket].current_user,"\0")==0)
	{
		echoMess(connSock, 22);
		return;
	}
	echoMess(connSock, 21);
	strcpy(sock[number_socket].current_user, "\0");
	return;
}
void exitSession(int number_socket)
{
	SOCKET connSock = sock[number_socket].connSock;
	//char current_account[BUFF_SIZE];
	//strcpy(current_account, sockNode.current_user);
	if (strcmp(sock[number_socket].current_user,"\0")!=0)
	{
		echoMess(connSock, 32);
		return;
	}
	echoMess(connSock, 31);
	return;
}
int receiveMess(SOCKET connSock, char *buff, int size, int flags) {
	int n;

	n = recv(connSock, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error receive: %d\n", WSAGetLastError());

	return n;
}

/* The send() wrapper function*/
int sendMess(SOCKET connSock, char *buff, int size, int flags) {
	int n;

	n = send(connSock, buff, size, flags);
	if (n == SOCKET_ERROR)
		printf("Error send: %d\n", WSAGetLastError());

	return n;
}
unsigned __stdcall Proccess(void* param)
{
	int number_socket = *(int*)param-1;
	node sockNode = sock[number_socket];
	SOCKET connSock = sockNode.connSock;
	char receiveMessage[BUFF_SIZE], sendMessage[BUFF_SIZE];
	acc* accList = getAccountFromFile();
	for (int i = 0; i < 3; i++)
		accList[i].countFail = 0;
	while (true)
	{
		int ret = receiveMess(connSock, receiveMessage, BUFF_SIZE, 0);
		if (ret < 0) break;
		receiveMessage[ret] = 0;
		char header[MAX_HEADER];
		for (int i = 0; i < 6; i++)
		{
			header[i] = receiveMessage[i];
		}
		header[6] = 0;
		if (strcmp(header, "LOGIN ") == 0)
		{
			char user[BUFF_SIZE];
			char pass[BUFF_SIZE];
			int position = 0, j = 0, k = 0;
			for (int i = 6; i < ret; i++)
			{
				if (receiveMessage[i] == ' ') position++;
				else if (position == 0) user[j++] = receiveMessage[i];
				else
				{
					pass[k++] = receiveMessage[i];
				}
			}
			user[j] = 0;
			pass[k] = 0;
			puts(user);
			puts(pass);
			
			login(user, pass,number_socket,accList);
		}
		else if (strcmp(header, "LOGOUT") == 0)
		{
			logout(number_socket);
		}
		else if (strcmp(header, "EXIT  ") == 0)
		{
			exitSession(number_socket);
		}
	}
	shutdown(connSock, SD_SEND);
	closesocket(connSock);
	return 0;
}

int echoMess(SOCKET connSock, int code)
{
	char message[BUFF_SIZE];
	switch (code)
	{
	case 11:
	{
		strcpy(message, "Login Successful");
		break;
	}
	case 12:
	{
		strcpy(message, "ID not exist");
		break;
	}
	case 13:
	{
		strcpy(message, "The password is incorrect");
		break;
	}
	case 14:
	{
		strcpy(message, "You are already logged in, you need to log out before logging in as different user");
		break;
	}
	case 15:
	{
		strcpy(message, "Account has blocked");
		break;
	}
	case 16:
	{
		strcpy(message, "Your login failed too many times. Your account is block");
		break;
	}
	case 17:
	{
		strcpy(message, "This account is already logged in on another device");
		break;
	}
	case 21:
	{
		strcpy(message, "Logout Successful");
		break;
	}
	case 22:
	{
		strcpy(message, "You hasn't logged in");
		break;
	}
	case 31:
	{
		strcpy(message, "OK");
		break;
	}
	case 32:
	{
		strcpy(message, "You need to log out before exit");
	}
	default: break;
	}
	int ret = sendMess(connSock, message, strlen(message), 0);
	return 0;
}
void updateAccList(acc* accList, int countLine)
{

	FILE *fp = fopen(fileName, "w");
	for (int i = 0; i<countLine; i++)
	{
		char info[BUFF_SIZE];
		strcpy(info, accList[i].username);
		strcat(info, " ");
		strcat(info, accList[i].password);
		strcat(info, " ");
		if (accList[i].status == '1') strcat(info, "1");
		else strcat(info, "0");
		if (i == countLine - 1) strcat(info, "\0");
		else strcat(info, "\n");
		fputs(info, fp);
	}
	fclose(fp);
}