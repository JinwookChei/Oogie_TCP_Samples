#include <iostream>
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


int main_()
{
	char host[] = "127.0.0.1";
	unsigned short port = 65456;

	WSADATA	wsaData;
	SOCKET hServerSocket = NULL;
	SOCKET hClientSocket = NULL;
	SOCKADDR_IN serverAddress, clientAddress;

	int sizeClientAddress = sizeof(SOCKADDR_IN);
	char* recvDatas = NULL;

	do
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			DEBUG_BREAK();
			break;
		}
		hServerSocket = socket(PF_INET, SOCK_STREAM, 0);
		if (hServerSocket == INVALID_SOCKET)
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

		memset(&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.s_addr = hostIP;
		serverAddress.sin_port = htons(port);

		try
		{
			if (bind(hServerSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
			{
				throw std::runtime_error("> bind() failed and program terminated\n");
				break;
			}
		}
		catch (const std::exception& ex)
		{
			std::cerr << "> bind() failed by exception: " << ex.what();
			break;
		}

		if (listen(hServerSocket, 10) == SOCKET_ERROR)
		{
			printf("> listen() failed and program terminated\n");
			DEBUG_BREAK();
			break;
		}

		hClientSocket = accept(hServerSocket, (SOCKADDR*)&clientAddress, &sizeClientAddress);
		if (hClientSocket == INVALID_SOCKET)
		{
			DEBUG_BREAK();
			break;
		}

		printf("> client connected by IP address %s with Port number %u\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

		recvDatas = (char*)malloc(BUFFER_SIZE);
		if (recvDatas == NULL)
		{
			DEBUG_BREAK();
			break;
		}

		bool isRunning = true;
		while (isRunning)
		{
			memset(recvDatas, 0, BUFFER_SIZE);
			const int recvLen = recv(hClientSocket, recvDatas, BUFFER_SIZE, 0);
			if (recvLen == -1)
			{
				DEBUG_BREAK();
				isRunning = false;
				break;
			}

			printf("> echoed: %s\n", recvDatas);

			// SendAll Start
			size_t accumulBytesSent = 0;
			while (accumulBytesSent < recvLen)
			{
				size_t bytesSent = send(hClientSocket, recvDatas + accumulBytesSent, recvLen - accumulBytesSent, 0);
				if (-1 == bytesSent)
				{
					isRunning = false;
					DEBUG_BREAK();
					break;
				}
				accumulBytesSent += bytesSent;
			}
			// SendAll End

			if (strcmp(recvDatas, "quit") == 0)
			{
				isRunning = false;
			}
		}

	} while (false);


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

	if (hServerSocket != NULL)
	{
		closesocket(hServerSocket);
		hServerSocket = NULL;
	}

	WSACleanup();

	return 0;
}



int main()
{
	printf("> echo - server is activated\n");
	main_();
	printf("> echo - server is de - activated\n");

	return 0;
}