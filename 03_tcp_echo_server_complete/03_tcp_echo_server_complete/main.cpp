#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>


#define BUFFER_SIZE 1024

class EchoServer
{
public:
	EchoServer(char host[], unsigned short port, int sizeBuffer)
		: host(host),
		port(port),
		sizeBuffer(sizeBuffer),
		hServerSocket(NULL),
		hClientSocket(NULL),
		serverAddress(),
		clientAddress(),
		sizeClientAddress(sizeof(SOCKADDR_IN)),
		recvDatas(nullptr)
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			__debugbreak();
			return;
		}
	}
	~EchoServer()
	{
		CleanUp();
	}

	void Run(int argc, char* argv[])
	{
		hServerSocket = socket(PF_INET, SOCK_STREAM, 0);
		if (hServerSocket == INVALID_SOCKET)
		{
			__debugbreak();
			CleanUp();
			return;
		}

		unsigned long hostIP = inet_addr(host);
		if (hostIP == INADDR_NONE)
		{
			__debugbreak();
			CleanUp();
			return;
		}

		memset(&serverAddress, 0, sizeof(serverAddress));
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.s_addr = hostIP;
		serverAddress.sin_port = htons(port);

		try
		{
			if (bind(hServerSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
			{
				std::cerr << "> bind() failed and program terminated" << std::endl;
				CleanUp();
				return;
			}
		}
		catch (const std::exception& ex)
		{
			std::cerr << "> bind() failed by exception: " << ex.what() << std::endl;
			CleanUp();
			return;
		}

		if (listen(hServerSocket, 10) == SOCKET_ERROR)
		{
			printf("> listen() failed and program terminated\n");	
			CleanUp();
			return;
		}

		hClientSocket = accept(hServerSocket, (SOCKADDR*)&clientAddress, &sizeClientAddress);
		if (hClientSocket == INVALID_SOCKET)
		{
			__debugbreak();
			CleanUp();
			return;
		}

		printf("> client connected by IP address %s with Port number %u\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

		recvDatas = (char*)malloc(1024);
		if (recvDatas == nullptr) {
			__debugbreak();
			CleanUp();
			return;
		}

		while (true)
		{
			memset(recvDatas, 0, BUFFER_SIZE);
			const int recvLen = recv(hClientSocket, recvDatas, BUFFER_SIZE, 0);
			if (recvLen == -1) 
			{
				__debugbreak();
				CleanUp();
				return;
			}

			printf("> echoed: %s\n", recvDatas);

			// SendAll Start
			size_t accumulBytesSent = 0;
			while (accumulBytesSent < recvLen)
			{
				size_t bytesSent = send(hClientSocket, recvDatas + accumulBytesSent, recvLen - accumulBytesSent, 0);
				if (-1 == bytesSent)
				{
					__debugbreak();
					CleanUp();
					return;
				}
				accumulBytesSent += bytesSent;
			}
			// SendAll End

			if (strcmp(recvDatas, "quit") == 0)
			{
				CleanUp();
				return;
			}
		}
	}

	void CleanUp()
	{
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
	}

private:
	const char* host;
	const unsigned short port;
	int sizeBuffer;
	WSADATA	wsaData;
	SOCKET hServerSocket, hClientSocket;
	SOCKADDR_IN serverAddress, clientAddress;
	int sizeClientAddress;
	char* recvDatas;
};


int main(int argc, char* argv[])
{
	char host[] = "127.0.0.1";
	unsigned short port = 65456;

	EchoServer* echoServer = new EchoServer(host, port, BUFFER_SIZE);
	if (echoServer == nullptr)
	{
		__debugbreak();
		return 0;
	}

	printf("> echo - server is activated\n");
	echoServer->Run(argc, argv);
	printf("> echo - client is de - activated");


	if (echoServer != nullptr)
	{
		echoServer->CleanUp();
		delete echoServer;
		echoServer = nullptr;
	}
	

	return 0;
}