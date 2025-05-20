#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")
#define BUFFER_SIZE 1024

class BaseRequestHandler
{
public:
	virtual void Handle(SOCKET hClientSocket, sockaddr_in clientAddress) = 0;
	virtual ~BaseRequestHandler() = default;
};

class MyTCPSocketHandler : public BaseRequestHandler {
public:
	MyTCPSocketHandler(int sizeBuffer)
		: sizeBuffer(sizeBuffer)
	{
		recvDatas = (char*)malloc(sizeBuffer);
		if (recvDatas == nullptr) {
			__debugbreak();
			CleanUp();
			return;
		}
	}
	~MyTCPSocketHandler()
	{
		CleanUp();
	}

	void Handle(SOCKET hClientSocket, sockaddr_in clientAddress) override {
		printf("> client connected by IP address %s with Port number %u\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

		while (true)
		{
			if (recvDatas == nullptr)
			{
				__debugbreak();
				CleanUp();
				return;
			}
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
			int accumulBytesSent = 0;
			while (accumulBytesSent < recvLen)
			{
				int bytesSent = send(hClientSocket, recvDatas + accumulBytesSent, recvLen - accumulBytesSent, 0);
				if (-1 == bytesSent)
				{
					__debugbreak();
					//CleanUp();
					return;
				}
				accumulBytesSent += bytesSent;
			}
			// SendAll End

			if (strcmp(recvDatas, "quit") == 0)
			{
				return;
			}
		}
	}

	void CleanUp()
	{
		if (recvDatas != NULL)
		{
			delete recvDatas;
			recvDatas = NULL;
		}
	}
private:
	char* recvDatas;
	int sizeBuffer;
};



class EchoServer
{
public:
	EchoServer(char host[], unsigned short port, BaseRequestHandler* handler)
		: host(host),
		port(port),
		handler(handler),
		hServerSocket(NULL),
		hClientSocket(NULL),
		serverAddress(),
		clientAddress(),
		sizeClientAddress(sizeof(SOCKADDR_IN))
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

	void ServeForever(int argc, char* argv[])
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

		while (true)
		{
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

			if (handler == nullptr)
			{
				__debugbreak();
				CleanUp();
				return;
			}

			handler->Handle(hClientSocket, clientAddress);
		}
	}

	void CleanUp()
	{
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
	BaseRequestHandler* handler;
	WSADATA	wsaData;
	SOCKET hServerSocket, hClientSocket;
	SOCKADDR_IN serverAddress, clientAddress;
	int sizeClientAddress;
};


int main(int argc, char* argv[])
{
	char host[] = "127.0.0.1";
	unsigned short port = 65456;

	MyTCPSocketHandler* myTCPSocketHandler = new MyTCPSocketHandler(BUFFER_SIZE);
	if (myTCPSocketHandler == nullptr)
	{
		__debugbreak();
		return 0;
	}

	EchoServer* echoServer = new EchoServer(host, port, myTCPSocketHandler);
	if (echoServer == nullptr)
	{
		__debugbreak();
		return 0;
	}

	printf("> echo - server is activated\n");
	echoServer->ServeForever(argc, argv);
	printf("> echo - client is de - activated");


	if (myTCPSocketHandler != nullptr)
	{
		delete myTCPSocketHandler;
		myTCPSocketHandler = nullptr;
	}

	if (echoServer != nullptr)
	{
		delete echoServer;
		echoServer = nullptr;
	}

	return 0;
}