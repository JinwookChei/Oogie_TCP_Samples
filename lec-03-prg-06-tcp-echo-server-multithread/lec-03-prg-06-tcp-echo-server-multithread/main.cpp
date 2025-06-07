#include "stdafx.h"
#include "MyTCPSocketHandler.h"
#include "MyThread.h"
#include "EchoServer.h"


int main()
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(167);
#endif  // _DEBUG

	const char* host = "127.0.0.1";
	const unsigned short port = 65456;

	printf("> echo - server is activated\n");

	EchoServer<MyTCPSocketHandler>* echoServer = new EchoServer<MyTCPSocketHandler>(host, port);
	if (echoServer == nullptr)
	{
		DEBUG_BREAK();
		return 0;
	}

	MyThread* serverThread = new MyThread(std::bind(&EchoServer<MyTCPSocketHandler>::ServeForever, echoServer));
	if (serverThread == nullptr) {
		DEBUG_BREAK();
		return 0;
	}

	serverThread->Start();

	printf("> server loop running in thread (main thread):%s\n", serverThread->GetThreadName());

	size_t baseThreadNumber = MyThread::ActiveCount();

	char msg[32];
	size_t len;
	while (true)
	{
		printf("> ");
		if (fgets(msg, sizeof(msg), stdin) == NULL)
		{
			break;
		}

		// 개행 문자 제거
		len = strlen(msg);
		if (len > 0 && msg[len - 1] == '\n')
		{
			msg[len - 1] = '\0';
		}

		if (strcmp(msg, "quit") == 0) {

			if (baseThreadNumber == MyThread::ActiveCount())
			{
				printf("> stop procedure started\n");
				break;
			}
			else
			{
				printf("> active threads are remained : %zu threads\n", MyThread::ActiveCount() - baseThreadNumber);
			}
		}
	}

	printf("> echo - server is de-activated\n");
	echoServer->ShutDown();	
	serverThread->Join();


	if (serverThread != nullptr)
	{
		delete serverThread;
		serverThread = nullptr;
	}

	if (echoServer != nullptr)
	{
		delete echoServer;
		echoServer = nullptr;
	}



	return 0;
}
