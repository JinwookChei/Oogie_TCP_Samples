#include "stdafx.h"
#include "MyTCPSocketHandler.h"
#include "EchoServer.h"

int main()
{
	const char* host = "127.0.0.1";
	const unsigned short port = 65456;

	EchoServer<MyTCPSocketHandler>* echoServer = new EchoServer<MyTCPSocketHandler>(host, port);
	if (echoServer == nullptr)
	{
		DEBUG_BREAK();
		return 0;
	}

	printf("> echo - server is activated\n");
	try {
		echoServer->ServeForever();
	}
	catch (const std::exception& ex) {
		std::cerr << "Exception: " << ex.what() << std::endl;
	}
	printf("> echo - server is de-activated\n");

	delete echoServer;
	return 0;
}
