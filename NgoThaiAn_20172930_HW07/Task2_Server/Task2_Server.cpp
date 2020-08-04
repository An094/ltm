#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <conio.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
#define WM_SOCKET WM_USER +1
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define MAX_CLIENT 1024
#define BUFF_SIZE 2048

#define DATA_SIZE 65
#define MAX_NODE 10
int mainProc(int);
int Send(SOCKET, char*, int, int);
int Receive(SOCKET, char*, int, int);
void processMessage(char*, int);
char encryption(char, int);
char decryption(char, int);
void receiveData(char*, int);
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


ATOM MyRegisterClass(HINSTANCE hInstance);
HWND InitInstance(HINSTANCE, int);
LRESULT CALLBACK windowProc(HWND, UINT, WPARAM, LPARAM);


SOCKET client[MAX_CLIENT];
SOCKET listenSock;

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
	serverAddr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		MessageBox(serverWindow, L"Cannot bind!", L"Error!", MB_OK);
	}

	//Listen request from client
	if (listen(listenSock, MAX_CLIENT)) {
		MessageBox(serverWindow, L"Cannot listen!", L"Error!", MB_OK);
		return 0;
	}

	//Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;

}
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

LRESULT CALLBACK windowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	SOCKET connSock;
	sockaddr_in clientAddr;
	int clientAddrLen = sizeof(clientAddr), i;
	//char rcvBuff[BUFF_SIZE], sendBuff[BUFF_SIZE];
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
					sock[i] = createNode(client[i], 0,0);
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
				if (client[i] == (SOCKET)wParam) {
					if (mainProc(i) < 0)
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

int mainProc(int number_sock)
{
	SOCKET connSock = sock[number_sock].connSock;
	char message[BUFF_SIZE];
	
	int ret = Receive(connSock, message, BUFF_SIZE, 0);
	if (ret < 0) return -1;
	Send(connSock, "OK", 2, 0);
	
	processMessage(message, number_sock);
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
void processMessage(char message[], int number_sock)
{
	if (message[0] == '0' || message[0] == '1')
	{

		int key = atoi(message + 3);
		sock[number_sock].key = key;
		sock[number_sock].mode = message[0] - 48;
	}
	else if (message[0] == '2')
	{
		receiveData(message, number_sock);
	}
}
char encryption(char message, int key)
{
	return (message + key) % 256;
}
char decryption(char message, int key)
{
	return (message - key) % 256;
}
void receiveData(char message[], int number_sock)
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
			fputc(encryption(temp, key), fpres);
		}
	}
	else
	{
		while (!feof(fp))
		{
			char temp = fgetc(fp);
			fputc(decryption(temp, key), fpres);
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

