#pragma once

class MyThread;
class BaseRequestHandler
{
public:
	BaseRequestHandler();

	virtual ~BaseRequestHandler();

	bool Init(SOCKET hClientSocket, SOCKADDR_IN clientAddress);
	
	void Execute();

	virtual unsigned int Handle() = 0;

	virtual void CleanUp();

protected:
	SOCKET hClientSocket_;
	SOCKADDR_IN clientAddress_;
	MyThread* thread_;
};
