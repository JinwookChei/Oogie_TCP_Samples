#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <Windows.h>
#include <map>
#include <memory>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")
#define BUFFER_SIZE 1024

class BaseRequestHandler
{
public:
	virtual unsigned int Handle(SOCKET hClientSocket, const sockaddr_in& clientAddress) = 0;
	virtual ~BaseRequestHandler() = default;
};

class MyTCPSocketHandler : public BaseRequestHandler {
public:
	MyTCPSocketHandler(int sizeBuffer)
		: sizeBuffer_(sizeBuffer)
	{
		recvDatas_ = std::make_unique<char[]>(sizeBuffer_);
	}
	~MyTCPSocketHandler() = default;

	unsigned int Handle(SOCKET hClientSocket, const sockaddr_in& clientAddress) override {
		char ipStr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(clientAddress.sin_addr), ipStr, INET_ADDRSTRLEN);

		printf("> client connected by IP address %s with Port number %u\n", ipStr, ntohs(clientAddress.sin_port));

		while (true)
		{
			memset(recvDatas_.get(), 0, sizeBuffer_);
			int recvLen = recv(hClientSocket, recvDatas_.get(), sizeBuffer_ - 1, 0);
			if (recvLen == 0) {
				printf("> client disconnected\n");
				break;
			}
			if (recvLen < 0) {
				printf("> recv() failed\n");
				break;
			}

			recvDatas_.get()[recvLen] = '\0';
			printf("> echoed: %s\n", recvDatas_.get());

			// SendAll
			int accumulBytesSent = 0;
			while (accumulBytesSent < recvLen)
			{
				int bytesSent = send(hClientSocket, recvDatas_.get() + accumulBytesSent, recvLen - accumulBytesSent, 0);
				if (bytesSent == SOCKET_ERROR)
				{
					printf("> send() failed\n");
					return -1;
				}
				accumulBytesSent += bytesSent;
			}

			if (strcmp(recvDatas_.get(), "quit") == 0)
			{
				break;
			}
		}
		return 0;
	}

private:
	std::unique_ptr<char[]> recvDatas_;
	int sizeBuffer_;
};

// 스레드 파라미터 구조체
struct ThreadParams {
	//EchoServer* server_;
	//unsigned int threadId_;
	SOCKET clientSocket_;
	sockaddr_in clientAddress_;
	BaseRequestHandler* handler_;
};

template<typename HandlerType>
class EchoServer
{
public:
	EchoServer(const char* host, unsigned short port, int sizeBuffer)
		: host_(host),
		port_(port),
		handler_(nullptr),
		hServerSocket_(INVALID_SOCKET),
		sizeBuffer_(sizeBuffer)
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

			ThreadParams* params = new ThreadParams;
			//params->server_ = this;
			params->clientSocket_ = clientSocket;
			params->clientAddress_ = clientAddress;
			params->handler_;
			//params->threadId_;

			unsigned int threadId = 0;
			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, EchoServer::ThreadFunc, params, 0, &threadId);

			if (hThread == 0)
			{
				printf("> _beginthreadex() failed\n");
				closesocket(clientSocket);
				delete params;
				continue;
			}

			{
				std::lock_guard<std::mutex> lock(threadMapMutex_);
				threads_[threadId] = hThread;
			}
		}
	}

	static unsigned int __stdcall ThreadFunc(void* param)
	{
		std::unique_ptr<ThreadParams> params(static_cast<ThreadParams*>(param));
		if (!params || !params->handler_ || !params->server_)
		{
			_endthreadex(-1);
			return -1;
		}

		unsigned int retCode = params->handler_->Handle(params->clientSocket_, params->clientAddress_);
		closesocket(params->clientSocket_);

		
		params->server_->RemoveThread(params->threadId_);

		_endthreadex(retCode);
		return retCode;
	}

	void RemoveThread(unsigned int threadId)
	{
		std::lock_guard<std::mutex> lock(threadMapMutex_);
		auto it = threads_.find(threadId);
		if (it != threads_.end())
		{
			WaitForSingleObject(it->second, INFINITE);
			CloseHandle(it->second);
			threads_.erase(it);
		}
	}

	void CleanUp()
	{
		{
			std::lock_guard<std::mutex> lock(threadMapMutex_);
			for (auto& th : threads_)
			{
				WaitForSingleObject(th.second, INFINITE);
				CloseHandle(th.second);
			}
			threads_.clear();
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
	//BaseRequestHandler* handler_;
	WSADATA wsaData_;
	SOCKET hServerSocket_;
	SOCKADDR_IN serverAddress_;
	int sizeBuffer_;
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
