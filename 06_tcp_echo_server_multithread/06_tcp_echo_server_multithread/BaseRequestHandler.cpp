#include "stdafx.h"
#include "BaseRequestHandler.h"

BaseRequestHandler::BaseRequestHandler()
	: hClientSocket_(NULL),
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
