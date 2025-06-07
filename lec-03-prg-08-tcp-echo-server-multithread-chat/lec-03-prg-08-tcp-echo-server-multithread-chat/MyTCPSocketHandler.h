#pragma once
#include "BaseRequestHandler.h"

class MyTCPSocketHandler : public BaseRequestHandler {
public:
	MyTCPSocketHandler();

	~MyTCPSocketHandler();

	unsigned int Handle() override;

	void CleanUp() override;

public:
	static std::vector<SOCKET> socketGroup_;

private:
	char* recvDatas_;
};