#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <algorithm>
#include <vector>

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
	virtual void Handle(SOCKET hSocket) = 0;
	virtual ~BaseRequestHandler() = default;
};

class MyUDPSocketHandler : public BaseRequestHandler {
public:
	MyUDPSocketHandler()
		: recvDatas_(nullptr),
		clientAddress_(),
		sizeClientAddress_()
	{
		recvDatas_ = (char*)malloc(BUFFER_SIZE);
		if (recvDatas_ == NULL) {
			DEBUG_BREAK();
			CleanUp();
			return;
		}
	}
	~MyUDPSocketHandler()
	{
		CleanUp();
	}

	void Handle(SOCKET hSocket) override
	{
		memset(recvDatas_, 0, BUFFER_SIZE);
		sizeClientAddress_ = (sizeof(SOCKADDR_IN));
		int recvLen = recvfrom(hSocket, recvDatas_, BUFFER_SIZE, 0, (SOCKADDR*)&clientAddress_, &sizeClientAddress_);
		if (recvLen == SOCKET_ERROR) {
			DEBUG_BREAK();
			CleanUp();
			return;
		}
		
		if (recvDatas_[0] == '#' || strcmp(recvDatas_, "quit") == 0)
		{
			if (strcmp(recvDatas_, "#REG") == 0)
			{
				printf("> client registered (%s, %u)\n", inet_ntoa(clientAddress_.sin_addr), ntohs(clientAddress_.sin_port));
				groupQueue_.push_back(clientAddress_);
			}
			else if (strcmp(recvDatas_, "#DEREG") == 0 || strcmp(recvDatas_, "quit"))
			{
				auto findIter = std::find_if(groupQueue_.begin(), groupQueue_.end(),
					[&](const SOCKADDR_IN& addr)
					{
						return addr.sin_family == clientAddress_.sin_family &&
							addr.sin_addr.s_addr == clientAddress_.sin_addr.s_addr &&
							addr.sin_port == clientAddress_.sin_port;
					});
				if (findIter != groupQueue_.end())
				{
					printf("> client de-registered (%s, %u)\n", inet_ntoa(clientAddress_.sin_addr), ntohs(clientAddress_.sin_port));
					groupQueue_.erase(findIter);
				}
			}
		}
		else
		{
			if (groupQueue_.size() == 0)
			{
				printf("> no clients to echo\n");
			}
			else
			{
				auto findIter = std::find_if(groupQueue_.begin(), groupQueue_.end(),
					[&](const SOCKADDR_IN& addr)
					{
						return addr.sin_family == clientAddress_.sin_family &&
							addr.sin_addr.s_addr == clientAddress_.sin_addr.s_addr &&
							addr.sin_port == clientAddress_.sin_port;
					});

				if (findIter == groupQueue_.end())
				{
					printf("> ignores a message from un-registered client\n");
				}
				else
				{
					printf("> received (%s) and echoed to %zu client\n", recvDatas_, groupQueue_.size());
					for (int i = 0; i < groupQueue_.size(); ++i)
					{
						int sentLen = sendto(hSocket, recvDatas_, recvLen, 0, (SOCKADDR*)&groupQueue_[i], sizeClientAddress_);
						if (sentLen == SOCKET_ERROR) {
							DEBUG_BREAK();
							CleanUp();
							return;
						}
					}
				}
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

	SOCKADDR_IN clientAddress_;

	int sizeClientAddress_;

	static std::vector<SOCKADDR_IN> groupQueue_;
};

std::vector<SOCKADDR_IN> MyUDPSocketHandler::groupQueue_;


template<typename HandlerType>
class UDPServer
{
	static_assert(std::is_base_of<BaseRequestHandler, HandlerType>::value, "HandlerType must inherit from BaseRequestHandler");

public:
	UDPServer(char host[], unsigned short port)
		: host_(host),
		port_(port),
		handler_(nullptr),
		hServerSocket_(NULL),
		serverAddress_()
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsaData_) != 0)
		{
			DEBUG_BREAK();
			return;
		}
	}
	~UDPServer()
	{
		CleanUp();
	}

	void ServeForever()
	{
		hServerSocket_ = socket(PF_INET, SOCK_DGRAM, 0);
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
				throw std::runtime_error("> bind() failed and program terminated\n");
				CleanUp();
				return;
			}
		}
		catch (const std::exception& ex)
		{
			std::cerr << "> bind() failed by exception: " << ex.what();
			CleanUp();
			return;
		}

		handler_ = new HandlerType;
		if (handler_ == nullptr)
		{
			DEBUG_BREAK();
			CleanUp();
			return;
		}

		while (true)
		{
			handler_->Handle(hServerSocket_);
		}
	}

	void CleanUp()
	{
		if (handler_ != nullptr)
		{
			delete handler_;
			handler_ = nullptr;
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
	WSADATA	wsaData_;
	SOCKET hServerSocket_;
	SOCKADDR_IN serverAddress_;
	BaseRequestHandler* handler_;
};


int main()
{
	char host[] = "127.0.0.1";
	unsigned short port = 65456;

	UDPServer<MyUDPSocketHandler>* udpServer = new UDPServer<MyUDPSocketHandler>(host, port);
	if (udpServer == nullptr)
	{
		DEBUG_BREAK();
		return 0;
	}

	printf("> echo - server is activated\n");
	udpServer->ServeForever();
	printf("> echo - client is de - activated");

	if (udpServer != nullptr)
	{
		delete udpServer;
		udpServer = nullptr;
	}

	return 0;
}