#include <stdio.h>
#include <stdlib.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>


#define BUFFER_SIZE 1024
static char host[] = "127.0.0.1";
unsigned short port = 65456;

void ErrorHandling(const char* message);

int main(int argc, char* argv[])
{
	WSADATA wsaData;
	SOCKET hClientSocket;
	SOCKADDR_IN servAddr;

	printf("> echo - client is activated\n");

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}

	hClientSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hClientSocket == INVALID_SOCKET)
	{
		ErrorHandling("socket() error");
	}

	unsigned long hostIP = inet_addr(host);
	if (hostIP == INADDR_NONE)
	{
		ErrorHandling("inet_addr() error");
	}

	memset(&servAddr, 0, sizeof(servAddr));
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.s_addr = hostIP;
	servAddr.sin_port = htons(port);

	if (connect(hClientSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
	{
		ErrorHandling("connect() error!");
	}

	char* sendDatas = (char*)malloc(BUFFER_SIZE);
	char* recvDatas = (char*)malloc(BUFFER_SIZE);

	if (sendDatas == NULL || recvDatas == NULL)
	{
		ErrorHandling("malloc failed");
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
		while (accumulBytesSent < sendLen) {
			int bytesSent = send(hClientSocket, sendDatas + accumulBytesSent, sendLen - accumulBytesSent, 0);
			if (bytesSent == SOCKET_ERROR) {
				ErrorHandling("send() error");
			}
			accumulBytesSent += bytesSent;
		}
		// SendAll End

		// 수신
		memset(recvDatas, 0, BUFFER_SIZE);
		int recvLen = recv(hClientSocket, recvDatas, BUFFER_SIZE - 1, 0);  // -1 for null-termination
		if (recvLen == -1) {
			ErrorHandling("recv() error or connection closed");
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

void ErrorHandling(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}