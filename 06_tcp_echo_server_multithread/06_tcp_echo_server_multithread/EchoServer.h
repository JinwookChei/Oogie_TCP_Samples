#pragma once
#include "stdafx.h"
#include "BaseRequestHandler.h"
#include "MyThread.h"


class BaseRequestHandler;

struct ClientInfo
{
	SOCKET clientSocket_;
	sockaddr_in clientAddress_;
	BaseRequestHandler* handler_;
	MyThread* thread_;
};

template<typename HandlerType>
class EchoServer
{
	static_assert(std::is_base_of<BaseRequestHandler, HandlerType>::value, "HandlerType must inherit from BaseRequestHandler");

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

			HandlerType* handler = new HandlerType;
			BaseRequestHandler* requestHandler = dynamic_cast<BaseRequestHandler*>(handler);
			if (requestHandler == nullptr)
			{
				printf("> requesthandler create failed\n");
				if (clientSocket != INVALID_SOCKET)
				{
					closesocket(clientSocket);
					clientSocket = NULL;
				}
				DEBUG_BREAK();
				continue;
			}

			if (requestHandler->Init(clientSocket, clientAddress) == false)
			{
				printf("> requesthandler init failed\n");
				if (requestHandler != nullptr)
				{
					delete requestHandler;
					requestHandler = nullptr;
				}
				if(clientSocket != INVALID_SOCKET)
				{
					closesocket(clientSocket);
					clientSocket = NULL;
				}
				DEBUG_BREAK();
				continue;
			}

			MyThread* newThread = new MyThread(std::bind(&BaseRequestHandler::Handle, requestHandler));
			if (newThread == nullptr)
			{
				printf("failed to create thread\n");
				if (requestHandler != nullptr)
				{
					delete requestHandler;
					requestHandler = nullptr;
				}
				if (clientSocket != INVALID_SOCKET)
				{
					closesocket(clientSocket);
					clientSocket = NULL;
				}
				DEBUG_BREAK();
				continue;
			}

			ClientInfo* clientInfo = new ClientInfo;
			clientInfo->clientSocket_ = clientSocket;
			clientInfo->clientAddress_ = clientAddress;
			clientInfo->handler_ = requestHandler;
			clientInfo->thread_ = newThread;
			clientInfoList.push_back(clientInfo);

			newThread->Start();
		}

		return 1;
	}

	void ShutDown()
	{
		CleanUp();
	}

private:
	void CleanUp()
	{
		for (int i = 0; i < clientInfoList.size(); ++i)
		{
			/*clientInfoList[i]->thread_;
			clientInfoList[i]->clientSocket_;
			clientInfoList[i]->clientAddress_;
			clientInfoList[i]->handler_;*/

			delete clientInfoList[i]->thread_;
			delete clientInfoList[i]->handler_;
			delete clientInfoList[i];
			clientInfoList[i] = nullptr;
		}

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
	std::vector<ClientInfo*> clientInfoList;
};

