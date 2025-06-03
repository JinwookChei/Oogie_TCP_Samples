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


int main(int argc, char* argv[])
{
	printf("> echo - server is activated\n");

	WSADATA	wsaData;
	SOCKET hServerSocket, hClientSocket;
	SOCKADDR_IN serverAddress, clientAddress;

	int sizeClientAddress = sizeof(clientAddress);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		DEBUG_BREAK();
		return -1;
	}

	hServerSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (hServerSocket == INVALID_SOCKET)
	{
		DEBUG_BREAK();
		return -1;
	}

	unsigned long hostIP = inet_addr(host);
	if (hostIP == INADDR_NONE)
	{
		DEBUG_BREAK();
		return -1;
	}

	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = hostIP;
	serverAddress.sin_port = htons(port);

	if (bind(hServerSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		DEBUG_BREAK();
		return -1;
	}

	if (listen(hServerSocket, 10) == SOCKET_ERROR)
	{
		DEBUG_BREAK();
		return -1;
	}

	hClientSocket = accept(hServerSocket, (SOCKADDR*)&clientAddress, &sizeClientAddress);
	if (hClientSocket == INVALID_SOCKET)
	{
		DEBUG_BREAK();
		return -1;
	}

	printf("> client connected by IP address %s with Port number %u\n",
		inet_ntoa(clientAddress.sin_addr),
		ntohs(clientAddress.sin_port));


	char* recvDatas = (char*)malloc(1024);
	if (recvDatas == NULL) {
		DEBUG_BREAK();
		return -1;
	}

	while (true)
	{
		memset(recvDatas, 0, BUFFER_SIZE);
		const int recvLen = recv(hClientSocket, recvDatas, BUFFER_SIZE, 0);
		if (recvLen == -1) {
			DEBUG_BREAK();
			return -1;
		}
		printf("> echoed: %s\n", recvDatas);

		// SendAll Start
		size_t accumulBytesSent = 0;
		while (accumulBytesSent < recvLen)
		{
			size_t bytesSent = send(hClientSocket, recvDatas+ accumulBytesSent, recvLen - accumulBytesSent, 0);
			if (-1 == bytesSent)
			{
				DEBUG_BREAK();
				return -1;
			}
			accumulBytesSent += bytesSent;
		}
		// SendAll End

		if (strcmp(recvDatas, "quit") == 0)
		{
			break;
		}
	}

	printf("> echo - server is de - activated\n");

	free(recvDatas);
	closesocket(hClientSocket);
	closesocket(hServerSocket);
	WSACleanup();
	return 0;
}