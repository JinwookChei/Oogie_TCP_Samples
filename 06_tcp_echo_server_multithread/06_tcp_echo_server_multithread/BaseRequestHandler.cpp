#include "stdafx.h"
#include "BaseRequestHandler.h"
#include "EchoServer.h"

BaseRequestHandler::BaseRequestHandler()
	: hClientSocket_(INVALID_SOCKET),
	clientAddress_()
{
}

BaseRequestHandler::~BaseRequestHandler()
{
	if (hClientSocket_ != INVALID_SOCKET) {
		closesocket(hClientSocket_);
		hClientSocket_ = INVALID_SOCKET;
	}
}

bool BaseRequestHandler::Init(SOCKET hClientSocket, SOCKADDR_IN clientAddress)
{
	hClientSocket_ = hClientSocket;
	clientAddress_ = clientAddress;

	return true;
}

unsigned __stdcall BaseRequestHandler::ThreadEntry(void* arg)
{
	BaseRequestHandler* requestHandler = static_cast<BaseRequestHandler*>(arg);
	requestHandler->Handle();
	requestHandler->Exit();
	return 0;
}

void BaseRequestHandler::Exit()
{

}
