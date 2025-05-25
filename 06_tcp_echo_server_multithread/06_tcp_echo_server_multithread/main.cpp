#include "stdafx.h"
#include "MyTCPSocketHandler.h"
#include "EchoServer.h"
#include "MyThread.h"


int main()
{
	const char* host = "127.0.0.1";
	const unsigned short port = 65456;

	printf("> echo - server is activated\n");

	EchoServer<MyTCPSocketHandler>* echoServer = new EchoServer<MyTCPSocketHandler>(host, port);
	if (echoServer == nullptr)
	{
		DEBUG_BREAK();
		return 0;
	}

	MyThread* serverThread = new MyThread(std::bind(EchoServer<MyTCPSocketHandler>::ServeForever, echoServer));
	if (serverThread == 0) {
		DEBUG_BREAK();
		return 0;
	}

	serverThread->Start();
	printf("> server loop running in thread (main thread):%u\n", serverThread->GetThreadName());

	printf("> echo - server is de-activated\n");


	if (serverThread != nullptr)
	{
		serverThread->Shutdown();

		delete serverThread;
		serverThread = nullptr;
	}

	if(echoServer != nullptr)
	{
		delete echoServer;
		echoServer = nullptr;
	}


	return 0;
}
