#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024

class EchoClient
{
public:
	EchoClient(char host[], unsigned short port, int sizeBuffer)
		: host_(host),
		port_(port),
		sizeBuffer_(sizeBuffer),
		hClientSocket_(NULL),
		servAddr_(),
		sendDatas_(nullptr),
		recvDatas_(nullptr)
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsaData_) != 0)
		{
			__debugbreak();
			return;
		}
	}

	~EchoClient()
	{
		CleanUp();
	}

	void Run()
	{
		hClientSocket_ = socket(PF_INET, SOCK_STREAM, 0);
		if (hClientSocket_ == INVALID_SOCKET)
		{
			__debugbreak();
			CleanUp();
			return;
		}

		unsigned long hostIP = inet_addr(host_);
		if (hostIP == INADDR_NONE)
		{
			__debugbreak();
			CleanUp();
			return;
		}

		memset(&servAddr_, 0, sizeof(servAddr_));
		servAddr_.sin_family = AF_INET;
		servAddr_.sin_addr.s_addr = hostIP;
		servAddr_.sin_port = htons(port_);

		try
		{
			if (connect(hClientSocket_, (SOCKADDR*)&servAddr_, sizeof(servAddr_)) == SOCKET_ERROR)
			{
				std::cerr << "> connect() failed and program terminated" << std::endl;
				CleanUp();
				return;
			}
		}
		catch (const std::exception& ex)
		{
			std::cerr << "> connect() failed by exception: " << ex.what() << std::endl;
			CleanUp();
			return;
		}

		sendDatas_ = (char*)malloc(BUFFER_SIZE);
		recvDatas_ = (char*)malloc(BUFFER_SIZE);

		if (sendDatas_ == NULL || recvDatas_ == NULL)
		{
			__debugbreak();
			return;
		}

		while (true) {
			memset(sendDatas_, 0, BUFFER_SIZE);
			printf("> ");
			fgets(sendDatas_, BUFFER_SIZE, stdin);  // 입력 받기 (개행 문자 포함)

			// 개행 문자 제거
			size_t sendLen = strlen(sendDatas_);
			if (sendLen > 0 && sendDatas_[sendLen - 1] == '\n') {
				sendDatas_[sendLen - 1] = '\0';
				--sendLen;
			}

			// SendAll Start
			size_t accumulBytesSent = 0;
			while (accumulBytesSent < sendLen)
			{
				size_t bytesSent = send(hClientSocket_, sendDatas_ + accumulBytesSent, sendLen - accumulBytesSent, 0);
				if (bytesSent == SOCKET_ERROR)
				{
					__debugbreak();
					return;
				}
				accumulBytesSent += bytesSent;
			}
			// SendAll End

			// 수신
			memset(recvDatas_, 0, BUFFER_SIZE);
			const int recvLen = recv(hClientSocket_, recvDatas_, BUFFER_SIZE, 0);
			if (recvLen == -1) {
				__debugbreak();
				return;
			}
			printf("> received: %s\n", recvDatas_);

			// 종료 조건
			if (strcmp(recvDatas_, "quit") == 0) {
				break;
			}
		}

	}

	void CleanUp() 
	{
		if (sendDatas_ != NULL)
		{
			free(sendDatas_);
			sendDatas_ = NULL;
		}

		if (recvDatas_ != NULL)
		{
			free(recvDatas_);
			recvDatas_ = NULL;
		}

		if (hClientSocket_ != NULL)
		{
			closesocket(hClientSocket_);
			hClientSocket_ = NULL;
		}
		WSACleanup();
	}

private:
	const char* host_;
	const unsigned short port_;
	int sizeBuffer_;

	WSADATA wsaData_;
	SOCKET hClientSocket_;
	SOCKADDR_IN servAddr_;

	char* sendDatas_;
	char* recvDatas_;
};


int main(int argc, char* argv[])
{
	char host[] = "127.0.0.1";
	unsigned short port = 65456;

	EchoClient* echoClient = new EchoClient(host, port, BUFFER_SIZE);
	if (echoClient == nullptr)
	{
		__debugbreak();
		return -1;
	}

	printf("> echo - client is activated\n");
	echoClient->Run();
	printf("> echo-client is de-activated\n");

	if (echoClient != nullptr)
	{
		echoClient->CleanUp();
		delete echoClient;
		echoClient = nullptr;
	}

	return 0;
}