#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>

#ifdef _DEBUG
#define DEBUG_BREAK() __debugbreak()
#else
#define DEBUG_BREAK() ((void)0)
#endif

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
	MyTCPSocketHandler(int sizeBuffer)	
		: sizeBuffer_(sizeBuffer)
	{
		recvDatas_ = (char*)malloc(sizeBuffer);
		if (recvDatas_ == nullptr) {
			DEBUG_BREAK();
			CleanUp();
			return;
		}
	}
	~MyTCPSocketHandler()
	{
		CleanUp();
	}

	void Handle(SOCKET hClientSocket, const sockaddr_in& clientAddress) override {
		printf("> client connected by IP address %s with Port number %u\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

		while (true)
		{
			if (recvDatas_ == nullptr)
			{
				DEBUG_BREAK();
				CleanUp();
				return;
			}
			memset(recvDatas_, 0, BUFFER_SIZE);
			const int recvLen = recv(hClientSocket, recvDatas_, BUFFER_SIZE, 0);
			if (recvLen == -1)
			{
				DEBUG_BREAK();
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
					DEBUG_BREAK();
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

class EchoServer
{
public:
	EchoServer(char host[], unsigned short port, BaseRequestHandler* handler)
		: host_(host),
		port_(port),
		handler_(handler),
		hServerSocket_(NULL),
		hClientSocket_(NULL),
		serverAddress_(),
		clientAddress_(),
		sizeClientAddress_(sizeof(SOCKADDR_IN))
	{	
		if (WSAStartup(MAKEWORD(2, 2), &wsaData_) != 0)
		{
			DEBUG_BREAK();
			return;
		}
	}
	~EchoServer()
	{
		CleanUp();
	}

	void ServeForever(int argc, char* argv[])
	{
		hServerSocket_ = socket(PF_INET, SOCK_STREAM, 0);
		if (hServerSocket_ == INVALID_SOCKET)
		{
			DEBUG_BREAK();
			CleanUp();
			return;
		}

		unsigned long hostIP = inet_addr(host_);
		if (hostIP == INADDR_NONE)
		{
			DEBUG_BREAK();
			CleanUp();
			return;
		}

		memset(&serverAddress_, 0, sizeof(serverAddress_));
		serverAddress_.sin_family = AF_INET;
		serverAddress_.sin_addr.s_addr = hostIP;
		serverAddress_.sin_port = htons(port_);

		try
		{
			if (bind(hServerSocket_, (SOCKADDR*)&serverAddress_, sizeof(serverAddress_)) == SOCKET_ERROR)
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

		if (listen(hServerSocket_, 10) == SOCKET_ERROR)
		{
			printf("> listen() failed and program terminated\n");
			CleanUp();
			return;
		}

		while (true)
		{
			hClientSocket_ = accept(hServerSocket_, (SOCKADDR*)&clientAddress_, &sizeClientAddress_);
			if (hClientSocket_ == INVALID_SOCKET)
			{
				DEBUG_BREAK();
				CleanUp();
				return;
			}

			if (handler_ == nullptr)
			{
				DEBUG_BREAK();
				CleanUp();
				return;
			}

			handler_->Handle(hClientSocket_, clientAddress_);
		}
	}

	void CleanUp()
	{
		if (hClientSocket_ != NULL)
		{
			closesocket(hClientSocket_);
			hClientSocket_ = NULL;
		}

		if (hServerSocket_ != NULL)
		{
			closesocket(hServerSocket_);
			hServerSocket_ = NULL;
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
};


int main(int argc, char* argv[])
{
	char host[] = "127.0.0.1";
	unsigned short port = 65456;

	BaseRequestHandler* handler = new MyTCPSocketHandler(BUFFER_SIZE);
	if (handler == nullptr)
	{
		DEBUG_BREAK();
		return 0;
	}

	EchoServer* echoServer = new EchoServer(host, port, handler);
	if (echoServer == nullptr)
	{
		DEBUG_BREAK();
		return 0;
	}

	printf("> echo - server is activated\n");
	echoServer->ServeForever(argc, argv);
	printf("> echo - client is de - activated");

	if (handler != nullptr)
	{
		delete handler;
		handler = nullptr;
	}

	if (echoServer != nullptr)
	{
		delete echoServer;
		echoServer = nullptr;
	}

	return 0;
}