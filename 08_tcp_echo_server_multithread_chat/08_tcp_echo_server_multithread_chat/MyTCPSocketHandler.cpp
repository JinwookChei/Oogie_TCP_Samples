#include "stdafx.h"
#include "MyThread.h"
#include "MyTCPSocketHandler.h"

std::vector<SOCKET*> MyTCPSocketHandler::socketGroup_;

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

	if (hClientSocket_ == NULL)
	{
		DEBUG_BREAK();
		CleanUp();
		return 0;
	}
	MyTCPSocketHandler::socketGroup_.push_back(&hClientSocket_);


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

		if (strcmp(recvDatas_, "quit") == 0)
		{
			for (auto it = MyTCPSocketHandler::socketGroup_.begin(); it != MyTCPSocketHandler::socketGroup_.end();) {
				if (*it == &this->hClientSocket_) {
					it = MyTCPSocketHandler::socketGroup_.erase(it);
				}
				else {
					++it;
				}
			}
			break;
		}
		else
		{
			printf("> received ( %s ) and echoed to %zu clients\n", recvDatas_, MyTCPSocketHandler::socketGroup_.size());
			for (int i = 0; i < MyTCPSocketHandler::socketGroup_.size(); ++i)
			{
				// SendAll Start
				int accumulBytesSent = 0;
				while (accumulBytesSent < recvLen)
				{
					int bytesSent = send(*MyTCPSocketHandler::socketGroup_[i], recvDatas_ + accumulBytesSent, recvLen - accumulBytesSent, 0);
					if (bytesSent == -1)
					{
						DEBUG_BREAK();
						break;
					}
					accumulBytesSent += bytesSent;
				}
				// SendAll End
			}
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
