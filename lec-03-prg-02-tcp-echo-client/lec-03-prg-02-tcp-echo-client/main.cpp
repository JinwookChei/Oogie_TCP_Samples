#include <stdio.h>
#include <stdlib.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#ifdef _DEBUG
#define DEBUG_BREAK() __debugbreak()
#else
#define DEBUG_BREAK() ((void)0)
#endif


#define BUFFER_SIZE 1024
static char host[] = "127.0.0.1";
unsigned short port = 65456;


int main()
{
	WSADATA wsaData;
	SOCKET hClientSocket = NULL;
	SOCKADDR_IN servAddr;

	char* sendDatas = NULL;
	char* recvDatas = NULL;

	do
	{
		printf("> echo - client is activated\n");

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			DEBUG_BREAK();
			break;
		}

		hClientSocket = socket(PF_INET, SOCK_STREAM, 0);
		if (hClientSocket == INVALID_SOCKET)
		{
			DEBUG_BREAK();
			break;
		}

		unsigned long hostIP = inet_addr(host);
		if (hostIP == INADDR_NONE)
		{
			DEBUG_BREAK();
			break;
		}

		memset(&servAddr, 0, sizeof(servAddr));
		servAddr.sin_family = AF_INET;
		servAddr.sin_addr.s_addr = hostIP;
		servAddr.sin_port = htons(port);

		if (connect(hClientSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
		{
			DEBUG_BREAK();
			break;
		}

		sendDatas = (char*)malloc(BUFFER_SIZE);
		recvDatas = (char*)malloc(BUFFER_SIZE);

		if (sendDatas == NULL || recvDatas == NULL)
		{
			DEBUG_BREAK();
			break;
		}

		bool isRunning = true;
		while (isRunning) {
			memset(sendDatas, 0, BUFFER_SIZE);
			printf("> ");
			fgets(sendDatas, BUFFER_SIZE, stdin);  // 입력 받기 (개행 문자 포함)

			// 개행 문자 제거
			size_t sendLen = strlen(sendDatas);
			if (sendLen > 0 && sendDatas[sendLen - 1] == '\n') {
				sendDatas[sendLen - 1] = '\0';
				--sendLen;
			}

			// SendAll Start
			size_t accumulBytesSent = 0;
			while (accumulBytesSent < sendLen)
			{
				size_t bytesSent = send(hClientSocket, sendDatas + accumulBytesSent, sendLen - accumulBytesSent, 0);
				if (bytesSent == SOCKET_ERROR)
				{
					DEBUG_BREAK();
					isRunning = false;
					break;
				}
				accumulBytesSent += bytesSent;
			}
			// SendAll End

			// 수신
			memset(recvDatas, 0, BUFFER_SIZE);
			const int recvLen = recv(hClientSocket, recvDatas, BUFFER_SIZE, 0);
			if (recvLen == -1) 
			{
				DEBUG_BREAK();
				isRunning = false;
				break;
			}
			printf("> received: %s\n", recvDatas);

			// 종료 조건
			if (strcmp(recvDatas, "quit") == 0) 
			{
				isRunning = false;
			}
		}

		printf("> echo-client is de-activated\n");

	} while (false);

	
	if (sendDatas != NULL)
	{
		free(sendDatas);
		sendDatas = NULL;
	}

	if (recvDatas != NULL)
	{
		free(recvDatas);
		recvDatas = NULL;
	}

	if (hClientSocket != NULL)
	{
		closesocket(hClientSocket);
		hClientSocket = NULL;
	}
	
	WSACleanup();

	return 0;
}