#include <stdio.h>
#include <stdlib.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024
static char host[] = "127.0.0.1";
unsigned short port = 65456;


int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hClientSocket;
	SOCKADDR_IN servAddr;

	printf("> echo - client is activated\n");

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		__debugbreak();
		return -1;
	}

	hClientSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hClientSocket == INVALID_SOCKET)
	{
		__debugbreak();
		return -1;
	}

	unsigned long hostIP = inet_addr(host);
	if (hostIP == INADDR_NONE)
	{
		__debugbreak();
		return -1;
	}

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = hostIP;
	servAddr.sin_port = htons(port);

	if (connect(hClientSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		__debugbreak();
		return -1;
	}

	char* sendDatas = (char*)malloc(BUFFER_SIZE);
	char* recvDatas = (char*)malloc(BUFFER_SIZE);

	if (sendDatas == NULL || recvDatas == NULL)
	{
		__debugbreak();
		return -1;
	}

	while (true) {
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
				__debugbreak();
				return -1;
			}
			accumulBytesSent += bytesSent;
		}
		// SendAll End

		// 수신
		memset(recvDatas, 0, BUFFER_SIZE);
		const int recvLen = recv(hClientSocket, recvDatas, BUFFER_SIZE, 0);
		if (recvLen == -1) {
			__debugbreak();
			return -1;
		}
		printf("> received: %s\n", recvDatas);

		// 종료 조건
		if (strcmp(recvDatas, "quit") == 0) {
			break;
		}
	}

	printf("> echo-client is de-activated\n");


	free(sendDatas);
	free(recvDatas);
	closesocket(hClientSocket);
	WSACleanup();
	return 0;
}