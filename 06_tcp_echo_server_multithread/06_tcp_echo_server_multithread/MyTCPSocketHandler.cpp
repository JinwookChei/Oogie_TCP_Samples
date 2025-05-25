#include "stdafx.h"
#include "MyTCPSocketHandler.h"

MyTCPSocketHandler::MyTCPSocketHandler()
{
	recvDatas_ = new char[BUFFER_SIZE];
}

MyTCPSocketHandler::~MyTCPSocketHandler()
{
	CleanUp();
}

unsigned int MyTCPSocketHandler::Handle()
{
	printf("> client connected by IP address %s with Port number %u\n", inet_ntoa(clientAddress_.sin_addr), ntohs(clientAddress_.sin_port));

	while (true)
	{
		if (recvDatas_ == nullptr)
		{
			DEBUG_BREAK();
			CleanUp();
			break;
		}
		memset(recvDatas_, 0, BUFFER_SIZE);
		const int recvLen = recv(hClientSocket_, recvDatas_, BUFFER_SIZE, 0);
		if (recvLen <= 0)
		{
			CleanUp();
			break;
		}

		printf("> echoed: %s\n", recvDatas_);

		// SendAll Start
		int accumulBytesSent = 0;
		while (accumulBytesSent < recvLen)
		{
			int bytesSent = send(hClientSocket_, recvDatas_ + accumulBytesSent, recvLen - accumulBytesSent, 0);
			if (bytesSent == -1)
			{
				DEBUG_BREAK();
				break;
			}
			accumulBytesSent += bytesSent;
		}
		// SendAll End

		if (strcmp(recvDatas_, "quit") == 0)
		{
			break;
		}
	}
	return 0;
}

void MyTCPSocketHandler::CleanUp()
{
	if (recvDatas_ != nullptr)
	{
		delete[] recvDatas_;
		recvDatas_ = nullptr;
	}
}
