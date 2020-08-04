#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#define _CRT_SECURE_NO_WARNINGS
#include <conio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <stdlib.h>

#define BUFF_SIZE 2048
#define MAX_ARRAY 50
#pragma comment(lib,"Ws2_32.lib")
typedef struct account {
	char username[BUFF_SIZE];
	char password[BUFF_SIZE];
	char status;
	int countFail;
}acc;
acc* accList = (acc*)malloc(MAX_ARRAY * sizeof(acc));

char current_acc[BUFF_SIZE] = "\0";
const char* fileName = "account.txt";
int reciveMessage(SOCKET);
int sendMessage(SOCKET, int);
void login(const char*,acc*, char*, char*,SOCKET);
void logout(SOCKET);
void exitSession(SOCKET);
void updateAccList(acc*,const char*,int);

char current_user[BUFF_SIZE];

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
	for (int i = 0; i < MAX_ARRAY; i++) accList[i].countFail = 0;
	while (true)
	{//accept request
		connSock = accept(listenSock, (sockaddr *)& clientAddr, &clientAddrLen);
		while (true) {
			//receive message from client
			int result = reciveMessage(connSock);
			if (result == SOCKET_ERROR)
			{
				printf("Error! Code: %d", WSAGetLastError());
				break;
			}
		} //end accepting
		closesocket(connSock);
	}
	//Step 5: Close socket
	closesocket(listenSock);

	//Step 6: Terminate Winsock
	WSACleanup();

	return 0;
}
int reciveMessage(SOCKET connSock)
{
	int ret;
	char buff[BUFF_SIZE];
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
		char prefix[BUFF_SIZE];
		for (int i = 0; i < 6; i++)
		{
			prefix[i] = buff[i];
		}
		prefix[6] = 0;
		if (strcmp(prefix, "LOGIN ") == 0)
		{
			char user[BUFF_SIZE];
			char pass[BUFF_SIZE];
			int position = 0,j=0,k=0;
			for (int i = 6; i < index; i++)
			{
				if (buff[i] == ' ') position++;
				else if (position == 0) user[j++] = buff[i];
				else
				{
					pass[k++] = buff[i];
				}
			}
			user[j] = 0;
			pass[k] = 0;
			puts(user);
			puts(pass);
			login(fileName,accList, user, pass,connSock);
		}
		else if(strcmp(prefix,"LOGOUT")==0)
		{
			logout(connSock);
			//printf("logout");
		}
		else if (strcmp(prefix, "EXIT  ") == 0)
		{
			exitSession(connSock);
		}
	}
	return 0;
}
int sendMessage(SOCKET connSock, int flag)
{
	char message[BUFF_SIZE];
	switch (flag)
	{
		case 11:
		{
			strcpy(message,"Login Successful");
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
		case 21:
		{
			strcpy(message, "Logout Successful");
			break;
		}
		case 22:
		{
			strcpy(message, "Account hasn't logged in");
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
	int ret = send(connSock, message, strlen(message), 0);
	if (ret == SOCKET_ERROR)
	{
		printf("Error! Code: %d", WSAGetLastError());
		return SOCKET_ERROR;
	}
	return 0;
}
void login(const char* filename, acc* accList,char* user,char* pass,SOCKET connSock)
{
	FILE *fp = fopen(filename, "r");
	//acc* accList=(acc*)malloc(MAX*sizeof(acc));
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

		strcpy(accList[countLine].username, userFile);
		strcpy(accList[countLine].password, passFile);
		accList[countLine].status = statusFile;

		countLine++;

	}
	fclose(fp);
	accList = (acc*)realloc(accList, countLine * sizeof(acc));
	if (strcmp(current_acc, "\0") != 0)
	{
		sendMessage(connSock, 14);
	}
	else
	{
		int i;
		for (i = 0; i < countLine; i++)
		{
			if (strcmp(accList[i].username, user) == 0)
			{
				if ((strcmp(accList[i].password, pass) == 0) && (accList[i].status == '0'))
				{
					int ret = sendMessage(connSock, 11);
					printf("Login Successful\n");
					accList[i].countFail = 0;
					ZeroMemory(current_acc, BUFF_SIZE);
					strcpy(current_acc, user);
					return;
				}
				else if (strcmp(accList[i].password, pass) != 0)
				{
					sendMessage(connSock, 13);
					accList[i].countFail++;
				}
				else if (accList[i].status == '1') sendMessage(connSock, 15);
				if (accList[i].countFail == 3)
				{
					printf("Account is block.\n");
					accList[i].status = '1';
				}
				break;
			}

		}
		updateAccList(accList, fileName, countLine);
		printf("Login Failed.\n");
		printf("Press to continue\n");
		if (i == countLine) sendMessage(connSock, 12);
	}
	return;
}
void logout(SOCKET connsock)
{
	if (strcmp(current_acc, "\0") == 0) sendMessage(connsock, 22);
	else
	{
		strcpy(current_acc, "\0");
		sendMessage(connsock, 21);
	}
}
void exitSession(SOCKET connSock)
{
	if (strcmp(current_acc, "\0") != 0) sendMessage(connSock, 32);
	else sendMessage(connSock, 31);
}
void updateAccList(acc* accList, const char *filename, int countLine)
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