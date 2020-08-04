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

node sock[FD_SETSIZE];
const char* fileName = "account.txt";

node createNode(SOCKET connSock, char user[])
{
	node p;
	p.connSock = connSock;
	strcpy(p.current_user, user);
	return p;
}
acc* getAccountFromFile();
int receiveMess(SOCKET, char*, int, int);
int sendMess(SOCKET, char*, int, int);
void login(char*, char*, int, acc*);
void logout(int);
void exitSession(int);
void updateAccList(acc*, int);
int echoMess(SOCKET, int);
int Process(int);

int amountOFAccountInFile;		//number of accounts in the file

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
	int nEvents, clientAddrLen;
	
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
						printf("new connection\n");
						client[i] = connSock;
						sock[i] = createNode(client[i], "\0");
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
				
				if (Process(i) < 0) {
					FD_CLR(client[i], &readfds);
					closesocket(client[i]);
					client[i] = 0;
				}
			}
			if (--nEvents <= 0) continue;
		}
	}
	_getch();
	return 0;
}
acc* getAccountFromFile()//return list account from file
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
void login(char* user, char* pass, int number_socket, acc* accList)
{
	SOCKET connSock = sock[number_socket].connSock;

	if (strcmp(sock[number_socket].current_user, "\0") != 0)   //check current user of socket 
	{
		echoMess(connSock, 14);
		return;
	}
	for (int i = 0; i <= FD_SETSIZE; i++)
	{
		if (strcmp(sock[i].current_user, user) == 0)
		{
			echoMess(connSock, 17);
			return;
		}
	}
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
	if (strcmp(sock[number_socket].current_user, "\0") == 0)//if current user is null
	{
		echoMess(connSock, 22);
		return;
	}
	echoMess(connSock, 21);
	strcpy(sock[number_socket].current_user, "\0");			//after logout current user of socket assign to null
	return;
}
void exitSession(int number_socket)
{
	SOCKET connSock = sock[number_socket].connSock;
	if (strcmp(sock[number_socket].current_user, "\0") != 0)	//check client was logout
	{
		echoMess(connSock, 32);
		return;
	}
	echoMess(connSock, 31);
	return;
}
int receiveMess(SOCKET connSock, char *buff, int size, int flags) {//The recv() wrapper function
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
int Process(int number_socket)
{
	node sockNode = sock[number_socket];
	SOCKET connSock = sockNode.connSock;
	char rcvMess[BUFF_SIZE], sendMess[BUFF_SIZE];
	acc* accList = getAccountFromFile();
	for (int i = 0; i < 3; i++)
		accList[i].countFail = 0;
	while (true)
	{
		int ret = receiveMess(connSock, rcvMess, BUFF_SIZE, 0);
		if (ret < 0) return -1;
		rcvMess[ret] = 0;
		char header[MAX_HEADER];
		for (int i = 0; i < 6; i++)
		{
			header[i] = rcvMess[i];
		}
		header[6] = 0;
		if (strcmp(header, "LOGIN ") == 0)
		{
			char user[BUFF_SIZE];
			char pass[BUFF_SIZE];
			int position = 0, j = 0, k = 0;
			for (int i = 6; i < ret; i++)
			{
				if (rcvMess[i] == ' ') position++;
				else if (position == 0) user[j++] = rcvMess[i];
				else
				{
					pass[k++] = rcvMess[i];
				}
			}
			user[j] = 0;
			pass[k] = 0;
			puts(user);
			puts(pass);

			login(user, pass, number_socket, accList);
			return 0;
		}
		else if (strcmp(header, "LOGOUT") == 0)
		{
			logout(number_socket);
			return 0;
		}
		else if (strcmp(header, "EXIT  ") == 0)
		{
			exitSession(number_socket);
			return 0;
		}
	}
	//shutdown(connSock, SD_SEND);
	//closesocket(connSock);
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
void updateAccList(acc* accList, int countLine)//after acc change, file acc is update
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