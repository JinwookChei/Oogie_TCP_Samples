#pragma once
#include "BaseRequestHandler.h"
#include <map>


template<typename HandlerType>
class EchoServer
{
	friend BaseRequestHandler;
public:
	EchoServer(const char* host, unsigned short port)
		: host_(host),
		port_(port),
		hServerSocket_(INVALID_SOCKET),
		serverAddress_()
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

	unsigned int ServeForever()
	{
		hServerSocket_ = socket(PF_INET, SOCK_STREAM, 0);
		if (hServerSocket_ == INVALID_SOCKET)
		{
			printf("> socket() failed\n");
			DEBUG_BREAK();
			CleanUp();
			return 0;
		}

		unsigned long hostIP = inet_addr(host_);
		if (hostIP == INADDR_NONE)
		{
			printf("> Invalid IP address\n");
			DEBUG_BREAK();
			CleanUp();
			return 0;
		}

		memset(&serverAddress_, 0, sizeof(serverAddress_));
		serverAddress_.sin_family = AF_INET;
		serverAddress_.sin_addr.s_addr = hostIP;
		serverAddress_.sin_port = htons(port_);

		if (bind(hServerSocket_, (SOCKADDR*)&serverAddress_, sizeof(serverAddress_)) == SOCKET_ERROR)
		{
			printf("> bind() failed\n");
			DEBUG_BREAK();
			CleanUp();
			return 0;
		}

		if (listen(hServerSocket_, 10) == SOCKET_ERROR)
		{
			printf("> listen() failed\n");
			DEBUG_BREAK();
			CleanUp();
			return 0;
		}

		while (true)
		{
			sockaddr_in clientAddress;
			int sizeClientAddress = sizeof(clientAddress);
			SOCKET clientSocket = accept(hServerSocket_, (SOCKADDR*)&clientAddress, &sizeClientAddress);
			if (clientSocket == INVALID_SOCKET)
			{
				printf("> accept() failed\n");
				DEBUG_BREAK();
				continue;
			}

			BaseRequestHandler* requestHandler = new HandlerType;
			if (requestHandler == nullptr)
			{
				printf("> requestHandler create failed\n");
				if (clientSocket != INVALID_SOCKET)
				{
					closesocket(clientSocket);
				}
				DEBUG_BREAK();
				continue;
			}

			if (requestHandler->Init(clientSocket, clientAddress) == false)
			{
				printf("> requestHandler Init failed\n");
				if (requestHandler != nullptr)
				{
					delete requestHandler;
				}
				if(clientSocket != INVALID_SOCKET)
				{
					closesocket(clientSocket);
				}
				DEBUG_BREAK();
				continue;
			}

			unsigned __int64  threadHandle = _beginthreadex(NULL, 0, &BaseRequestHandler::ThreadEntry, requestHandler, 0, NULL);
			if (threadHandle == 0) {
				
				printf("Failed to create thread\n");
				if (requestHandler != nullptr)
				{
					delete requestHandler;
				}
				if (clientSocket != INVALID_SOCKET)
				{
					closesocket(clientSocket);
				}
				DEBUG_BREAK();
				continue;
			}
		}

		return 1;
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

