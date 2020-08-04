// client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<conio.h>
#include<WinSock2.h>
#include<WS2tcpip.h>
#include<stdlib.h>

#define BUFF_SIZE 2048

#pragma comment(lib,"Ws2_32.lib")

void display(char *,char*);

int main(int argc, char* argv[])
{
	int SERVER_PORT = atoi(argv[2]);//convert *char to int
	char* SERVER_ADDR = argv[1];
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is a not supported\n");
	printf("Client started!\n");
	//Step2: Construct socket
	SOCKET client;
	client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	

	//Step3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
	


	//Step 4: Communicate with server
	char buff[BUFF_SIZE];
	int ret, serverAddrLen = sizeof(serverAddr);

	while(1)
	{
		
		printf("\nSend to server: ");
		gets_s(buff, BUFF_SIZE);
		if (buff == NULL)
			break;//break loop if buff is NULL(condition end loop)
		ret = sendto(client, buff, strlen(buff), 0, (SOCKADDR*)&serverAddr, serverAddrLen);
		if (ret == SOCKET_ERROR)
			printf("Error: %d", WSAGetLastError());

		
		ret = recvfrom(client, buff, BUFF_SIZE, 0, (SOCKADDR*)&serverAddr, &serverAddrLen);
		if (ret == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAETIMEDOUT)
				printf("Time-out!");
			else printf("Error! Cannot receive message");
		}
		else if (strlen(buff) > 0)
		{
			buff[ret] = 0;
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
	closesocket(client);
	WSACleanup();
	return 0;

}
void display(char *buff,char *kind)
{
	int bufflen = strlen(buff);
	for (int i = 1; i < bufflen; i++)
	{

		//message ={'h or i','A','IP or name official','B','Alias','C',...}.
		if (buff[i] == 'A') printf("\nOfficial %s: ",kind);//if kind is IP print Official IP.
		else if (buff[i] =='B') printf("\nAlias: ");
		else if (buff[i] > 'B'&&buff[i] < 'Z') printf("\n");
		else printf("%c", buff[i]);
	}
}