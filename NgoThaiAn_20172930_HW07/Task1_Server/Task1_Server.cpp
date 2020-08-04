#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#define WM_SOCKET WM_USER +1
#define SERVER_PORT 5500
#define MAX_CLIENT 1024
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

node sock[MAX_CLIENT];
const char* fileName = "account.txt";



// FUNCTION createNode(SOCKET,char*)
// PURPOSE: create a Node with socket and user.

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
int Process(int,acc*);

int amountOFAccountInFile;		//number of accounts in the file
ATOM MyRegisterClass(HINSTANCE hInstance);
HWND InitInstance(HINSTANCE, int);
LRESULT CALLBACK windowProc(HWND, UINT, WPARAM, LPARAM);

SOCKET client[MAX_CLIENT];
SOCKET listenSock;


acc* accList = getAccountFromFile();

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreveInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	HWND serverWindow;

	//Registering the Window Class
	MyRegisterClass(hInstance);

	//Create the window
	if ((serverWindow = InitInstance(hInstance, nCmdShow)) == NULL)
		return FALSE;
	//Initiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		printf("Version is not supported\n");

	//Construct socket	
	SOCKET listenSock;
	listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//requests Windows message-based notification of network events for listenSock
	WSAAsyncSelect(listenSock, serverWindow, WM_SOCKET, FD_ACCEPT | FD_CLOSE | FD_READ);

	//Bind address to socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		MessageBox(serverWindow, L"Cannot bind!", L"Error!", MB_OK);
	}

	//Listen request from client
	if (listen(listenSock, MAX_CLIENT)) {
		MessageBox(serverWindow, L"Cannot listen!", L"Error!", MB_OK);
		return 0;
	}
	
	for (int i = 0; i < amountOFAccountInFile; i++)
		accList[i].countFail = 0;
	//Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;

}
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = windowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	//wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WSAASYNCSELECTSERVER));
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"WindowClass";
	//wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	wcex.hIconSm = NULL;
	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
HWND InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;
	int i;
	for (i = 0; i < MAX_CLIENT; i++)
		client[i] = 0;
	hWnd = CreateWindow(L"WindowClass", L"WSAAsyncSelect TCP Server", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
	if (!hWnd)
		return FALSE;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return hWnd;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_SOCKET	- process the events on the sockets
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SOCKET connSock;
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr), i;
	
	switch (message)
	{
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lParam)) {
			for (i = 0; i<MAX_CLIENT; i++)
				if (client[i] == (SOCKET)wParam) {
					closesocket(client[i]);
					client[i] = 0;
					continue;
				}
		}
		switch (WSAGETSELECTEVENT(lParam)) {
		case FD_ACCEPT:
		{
			connSock = accept((SOCKET)wParam, (sockaddr*)&clientAddr, &clientAddrLen);
			if (connSock == INVALID_SOCKET)
				break;
			for (i = 0; i < MAX_CLIENT; i++)
				if (client[i] == 0) {
					client[i] = connSock;
					sock[i] = createNode(client[i], "\0");
					break;
					//requests Windows message-based notification of network events for listendSock
					WSAAsyncSelect(client[i], hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
				}
			if (i == MAX_CLIENT)
				MessageBox(hWnd, L"Too many clients!", L"Notice", MB_OK);
		}
		break;
		case FD_READ:
		{
			for (i = 0; i < MAX_CLIENT; i++)
				if (client[i] == (SOCKET)wParam){
					if (Process(i,accList) < 0)
					{
						closesocket(client[i]);
						client[i] = 0;
					}
					break;
				}
			
		}
		break;
		case FD_CLOSE:
		{
			for (i = 0; i<MAX_CLIENT; i++)
				if (client[i] == (SOCKET)wParam) {
					closesocket(client[i]);
					client[i] = 0;
					break;
				}
		}
		break;
		}
	}
	break;
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		shutdown(listenSock, SD_BOTH);
		closesocket(listenSock);
		WSACleanup();
		return 0;
	}
	break;
	case WM_CLOSE:
	{
		DestroyWindow(hWnd);
		shutdown(listenSock, SD_BOTH);
		closesocket(listenSock);
		WSACleanup();
		return 0;
	}
	break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

// FUNCTION: getAccountFromFile()
//
// PURPOSE: take account from the file
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

/*
	FUNCTION: login(char*,char*,int,acc*)
	PURPOSE: Verify that the password account from the client is the same as in the file
	number_socket: number of client in list client
*/
void login(char* user, char* pass, int number_socket, acc* accList)
{
	SOCKET connSock = sock[number_socket].connSock;

	if (strcmp(sock[number_socket].current_user, "\0") != 0)   //check current user of socket 
	{
		echoMess(connSock, 14);
		return;
	}
	for (int i = 0; i < MAX_CLIENT; i++)
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
/*
	FUNCTION: logout(int)
	PURPOSE: logout account and send message to client
*/
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

/*
	FUNCTION: exitSession(int)
	PURPOSE: if account was log out, user could end session
*/
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
// The recv() wrapper function
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

/*
	FUNCTION: Process(int,acc*)
	PURPOSE: receive message from client
			 check header: if header is LOGIN call function login
						   if header is LOGOUT call function logout
						   if header is EXIT call function exitSession
*/
int Process(int number_socket,acc* accList)
{
	node sockNode = sock[number_socket];
	SOCKET connSock = sockNode.connSock;
	char rcvMess[BUFF_SIZE];
	
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
	return 0;
}

/*
	FUNCTION: echoMess(SOCKET,int)
	PURPOSE: Send code-based messages from functions
*/
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

/*
	FUNCTION: updateAccList(acc*,int)
	PURPOSE: update file account if account blocked
*/
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