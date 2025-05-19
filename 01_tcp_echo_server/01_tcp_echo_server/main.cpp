#include <stdio.h>
#include <stdlib.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

void ErrorHandling(const char* message);

static char host[] = "127.0.0.1";
unsigned short port = 65456;

int main(int argc, char* argv[])
{
	printf("> echo - server is activated\n");

	WSADATA	wsaData;
	SOCKET hServerSocket, hClientSocket;
	SOCKADDR_IN serverAddress, clientAddress;

	int sizeClientAddress = sizeof(clientAddress);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		ErrorHandling("WSAStartup() error!");
	}

	hServerSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hServerSocket == INVALID_SOCKET)
	{
		ErrorHandling("socket() error");
	}

	unsigned long hostIP = inet_addr(host);
	if (hostIP == INADDR_NONE)
	{
		ErrorHandling("inet_addr() error");
	}

	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = hostIP;
	serverAddress.sin_port = htons(port);

	if (bind(hServerSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		ErrorHandling("bind() error");
	}

	if (listen(hServerSocket, 10) == SOCKET_ERROR)
	{
		ErrorHandling("listen() error");
	}

	hClientSocket = accept(hServerSocket, (SOCKADDR*)&clientAddress, &sizeClientAddress);
	if (hClientSocket == INVALID_SOCKET)
	{
		ErrorHandling("accept() error");
	}

	printf("> client connected by IP address %lu with Port number %u\n", 
		clientAddress.sin_addr.s_addr,
		clientAddress.sin_port);

	while (true)
	{
		char recvDatas[1024];
		recv(hClientSocket, recvDatas, sizeof(recvDatas), 0);
		printf("> echoed: %s\n", recvDatas);

		// SendAll Start
		size_t totalSizeSent = 0;
		while (totalSizeSent < sizeof(recvDatas))
		{
			size_t bytesSent = send(hClientSocket, recvDatas, sizeof(recvDatas), 0);
			if (-1 == bytesSent)
			{
				ErrorHandling("accept() error");
			}
			totalSizeSent += bytesSent;
		}
		// SendAll End

		if ("quit" == recvDatas)
		{
			break;
		}
	}

	printf("> echo - server is de - activated\n");
	closesocket(hClientSocket);
	closesocket(hServerSocket);
	WSACleanup();
	return 0;
}

void ErrorHandling(const char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
