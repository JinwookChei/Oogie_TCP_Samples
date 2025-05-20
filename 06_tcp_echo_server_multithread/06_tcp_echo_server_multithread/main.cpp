#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <process.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")
#define BUFFER_SIZE 1024

class BaseRequestHandler
{
public:
	virtual void Handle(SOCKET hClientSocket, const sockaddr_in& clientAddress) = 0;
	virtual ~BaseRequestHandler() = default;
};

class MyTCPSocketHandler : public BaseRequestHandler {
public:
	MyTCPSocketHandler()
		: sizeBuffer_(0),
		recvDatas_(nullptr)
	{

	}
	~MyTCPSocketHandler()
	{
		CleanUp();
	}

	bool Initialize(int sizeBuffer)
	{
		sizeBuffer_ = sizeBuffer;
		recvDatas_ = (char*)malloc(sizeBuffer);

		if (recvDatas_ == nullptr) {
			__debugbreak();
			CleanUp();
			return false;
		}

		return true;
	}

	void Handle(SOCKET hClientSocket, const sockaddr_in& clientAddress) override {
		printf("> client connected by IP address %s with Port number %u\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

		while (true)
		{
			if (recvDatas_ == nullptr)
			{
				__debugbreak();
				CleanUp();
				return;
			}
			memset(recvDatas_, 0, BUFFER_SIZE);
			const int recvLen = recv(hClientSocket, recvDatas_, BUFFER_SIZE, 0);
			if (recvLen == -1)
			{
				__debugbreak();
				CleanUp();
				return;
			}

			printf("> echoed: %s\n", recvDatas_);

			// SendAll Start
			int accumulBytesSent = 0;
			while (accumulBytesSent < recvLen)
			{
				int bytesSent = send(hClientSocket, recvDatas_ + accumulBytesSent, recvLen - accumulBytesSent, 0);
				if (bytesSent == -1)
				{
					__debugbreak();
					return;
				}
				accumulBytesSent += bytesSent;
			}
			// SendAll End

			if (strcmp(recvDatas_, "quit") == 0)
			{
				return;
			}
		}
	}

	void CleanUp()
	{
		if (recvDatas_ != NULL)
		{
			free(recvDatas_);
			recvDatas_ = NULL;
		}
	}
private:
	char* recvDatas_;
	int sizeBuffer_;
};

template <typename HandlerType>
class EchoServer
{
public:
	EchoServer(char host[], unsigned short port, int bufferSize)
		: host_(host),
		port_(port),
		handler_(nullptr),
		hServerSocket_(NULL),
		hClientSocket_(NULL),
		serverAddress_(),
		clientAddress_(),
		sizeClientAddress_(sizeof(SOCKADDR_IN)),
		hThread_(nullptr)
	{
		handler_ = new HandlerType;
		


		if (handler == nullptr)
		{
			__debugbreak();
			return;
		}

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

		if (listen(hServerSocket, 10) == SOCKET_ERROR)
		{
			printf("> listen() failed and program terminated\n");
			CleanUp();
			return;
		}

		while (true)
		{
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
		if (handler != nullptr)
		{
			delete handler;
			handler = nullptr;
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
	const char* host_;
	const unsigned short port_;
	BaseRequestHandler* handler_;
	WSADATA	wsaData_;
	SOCKET hServerSocket_, hClientSocket_;
	SOCKADDR_IN serverAddress_, clientAddress_;
	int sizeClientAddress_;

	//HANDLE hThread_;
};


int main(int argc, char* argv[])
{
	char host[] = "127.0.0.1";
	unsigned short port = 65456;

	EchoServer<MyTCPSocketHandler>* echoServer = new EchoServer<MyTCPSocketHandler>(host, port, BUFFER_SIZE);
	if (echoServer == nullptr)
	{
		__debugbreak();
		return 0;
	}

	printf("> echo - server is activated\n");
	echoServer->ServeForever(argc, argv);
	printf("> echo - client is de - activated");

	if (echoServer != nullptr)
	{
		delete echoServer;
		echoServer = nullptr;
	}

	return 0;
}