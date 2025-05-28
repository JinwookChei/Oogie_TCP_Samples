#include "stdafx.h"
#include "BaseRequestHandler.h"
#include "MyThread.h"

BaseRequestHandler::BaseRequestHandler()
	: hClientSocket_(NULL),
	clientAddress_(),
	thread_(nullptr)
{
}

BaseRequestHandler::~BaseRequestHandler()
{
	CleanUp();
}

bool BaseRequestHandler::Init(SOCKET hClientSocket, SOCKADDR_IN clientAddress)
{
	hClientSocket_ = hClientSocket;
	clientAddress_ = clientAddress;

	MyThread* newThread = new MyThread(std::bind(&BaseRequestHandler::Handle, this));
	if (newThread == nullptr)
	{
		printf("failed to create thread\n");
		DEBUG_BREAK();

		return false;
	}

	thread_ = newThread;

	return true;
}

void BaseRequestHandler::Execute()
{
	if (thread_ == nullptr)
	{
		DEBUG_BREAK();
		return;
	}

	thread_->Start();
}

void BaseRequestHandler::CleanUp()
{
	if (hClientSocket_ != INVALID_SOCKET) {
		closesocket(hClientSocket_);
		hClientSocket_ = INVALID_SOCKET;
	}

	if (thread_ != nullptr)
	{
		delete thread_;
		thread_ = nullptr;
	}
}
