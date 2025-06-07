#pragma once
#include "BaseRequestHandler.h"

class MyTCPSocketHandler : public BaseRequestHandler {
public:
	MyTCPSocketHandler();

	~MyTCPSocketHandler();

	unsigned int Handle() override;

	void CleanUp();

private:
	char* recvDatas_;
};