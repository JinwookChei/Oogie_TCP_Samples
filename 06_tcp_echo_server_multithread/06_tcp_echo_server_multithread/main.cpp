#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <Windows.h>


#pragma comment(lib, "ws2_32.lib")
#define BUFFER_SIZE 1024


class BaseRequestHandler
{
public:
	BaseRequestHandler()
		: hClientSocket_(INVALID_SOCKET),
		clientAddress_()
	{
	}

	virtual ~BaseRequestHandler() = default;

	void Init(SOCKET hClientSocket, SOCKADDR_IN clientAddress)
	{
		hClientSocket_ = hClientSocket;
		clientAddress_ = clientAddress;
	}

	virtual unsigned int Handle() = 0;

	static unsigned __stdcall ThreadEntry(void* arg) {
		BaseRequestHandler* requestHandler = static_cast<BaseRequestHandler*>(arg);
		requestHandler->Handle();
		return 0;
	}

protected:
	SOCKET hClientSocket_;
	SOCKADDR_IN clientAddress_;
};

class MyTCPSocketHandler : public BaseRequestHandler {
public:
	MyTCPSocketHandler() {
		recvDatas_ = new char[BUFFER_SIZE];
	}

	~MyTCPSocketHandler() {
	}

	unsigned int Handle() override {
		printf("> client connected by IP address %s with Port number %u\n", inet_ntoa(clientAddress_.sin_addr), ntohs(clientAddress_.sin_port));

		while (true)
		{
			if (recvDatas_ == nullptr)
			{
				__debugbreak();
				CleanUp();
				return 0;
			}
			memset(recvDatas_, 0, BUFFER_SIZE);
			const int recvLen = recv(hClientSocket_, recvDatas_, BUFFER_SIZE, 0);
			if (recvLen == -1)
			{
				__debugbreak();
				CleanUp();
				return 0;
			}

			printf("> echoed: %s\n", recvDatas_);

			// SendAll Start
			int accumulBytesSent = 0;
			while (accumulBytesSent < recvLen)
			{
				int bytesSent = send(hClientSocket_, recvDatas_ + accumulBytesSent, recvLen - accumulBytesSent, 0);
				if (bytesSent == -1)
				{
					__debugbreak();
					return 0;
				}
				accumulBytesSent += bytesSent;
			}
			// SendAll End

			if (strcmp(recvDatas_, "quit") == 0)
			{
				break;
			}
		}

		return 1;
	}

	void CleanUp()
	{
		if (recvDatas_ != nullptr)
		{
			delete[] recvDatas_;
			recvDatas_ = nullptr;
		}

	}
private:
	char* recvDatas_;
};

template<typename HandlerType>
class EchoServer
{
public:
	EchoServer(const char* host, unsigned short port)
		: host_(host),
		port_(port),
		hServerSocket_(INVALID_SOCKET),
		serverAddress_()
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsaData_) != 0)
		{
			__debugbreak();
			printf("WSAStartup failed\n");
		}
	}
	~EchoServer()
	{
		CleanUp();
	}

	void ServeForever()
	{
		hServerSocket_ = socket(PF_INET, SOCK_STREAM, 0);
		if (hServerSocket_ == INVALID_SOCKET)
		{
			printf("> socket() failed\n");
			CleanUp();
			return;
		}

		unsigned long hostIP = inet_addr(host_);
		if (hostIP == INADDR_NONE)
		{
			printf("> Invalid IP address\n");
			CleanUp();
			return;
		}

		memset(&serverAddress_, 0, sizeof(serverAddress_));
		serverAddress_.sin_family = AF_INET;
		serverAddress_.sin_addr.s_addr = hostIP;
		serverAddress_.sin_port = htons(port_);

		if (bind(hServerSocket_, (SOCKADDR*)&serverAddress_, sizeof(serverAddress_)) == SOCKET_ERROR)
		{
			printf("> bind() failed\n");
			CleanUp();
			return;
		}

		if (listen(hServerSocket_, 10) == SOCKET_ERROR)
		{
			printf("> listen() failed\n");
			CleanUp();
			return;
		}

		while (true)
		{
			sockaddr_in clientAddress;
			int sizeClientAddress = sizeof(clientAddress);
			SOCKET clientSocket = accept(hServerSocket_, (SOCKADDR*)&clientAddress, &sizeClientAddress);
			if (clientSocket == INVALID_SOCKET)
			{
				printf("> accept() failed\n");
				continue;
			}


			BaseRequestHandler* requestHandler = new HandlerType;
			if (requestHandler == nullptr)
			{
				printf("> requestHandler create failed\n");
				throw std::runtime_error("");
			}
			requestHandler->Init(clientSocket, clientAddress);

			HANDLE threadHandle = INVALID_HANDLE_VALUE;
			threadHandle = (HANDLE)_beginthreadex(NULL, 0, &BaseRequestHandler::ThreadEntry, requestHandler, 0, NULL);

			if (threadHandle == 0) {
				printf("Failed to create thread\n");
				delete requestHandler;
			}
			else {
				CloseHandle(threadHandle);
			}
		}
	}

	void CleanUp()
	{
		if (hServerSocket_ != INVALID_SOCKET)
		{
			closesocket(hServerSocket_);
			hServerSocket_ = INVALID_SOCKET;
		}
		WSACleanup();
	}

private:
	const char* host_;
	const unsigned short port_;
	WSADATA wsaData_;
	SOCKET hServerSocket_;
	SOCKADDR_IN serverAddress_;
};

int main()
{
	const char* host = "127.0.0.1";
	const unsigned short port = 65456;

	EchoServer<MyTCPSocketHandler>* echoServer = new EchoServer<MyTCPSocketHandler>(host, port);
	if (echoServer == nullptr)
	{
		__debugbreak();
		return 0;
	}

	printf("> echo - server is activated\n");
	try {
		echoServer->ServeForever();
	}
	catch (const std::exception& ex) {
		std::cerr << "Exception: " << ex.what() << std::endl;
	}
	printf("> echo - server is de-activated\n");

	delete echoServer;
	return 0;
}