#pragma once

class BaseRequestHandler
{
public:
	BaseRequestHandler();

	virtual ~BaseRequestHandler();

	bool Init(SOCKET hClientSocket, SOCKADDR_IN clientAddress);

	virtual unsigned int Handle() = 0;

protected:
	SOCKET hClientSocket_;
	SOCKADDR_IN clientAddress_;
};
