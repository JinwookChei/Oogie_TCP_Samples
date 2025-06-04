#pragma once
#include "BaseRequestHandler.h"

class MyTCPSocketHandler : public BaseRequestHandler {
public:
	MyTCPSocketHandler();

	~MyTCPSocketHandler();

	unsigned int Handle() override;

	void CleanUp();

public:
	static std::vector<SOCKET*> socketGroup_;

private:
	char* recvDatas_;
};