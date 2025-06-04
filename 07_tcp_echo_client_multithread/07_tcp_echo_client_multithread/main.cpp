#include "stdafx.h"
#include "MyThread.h"

#define BUFFER_SIZE 1024

unsigned int recvHandler(SOCKET hClientSocket)
{
	char* recvDatas = NULL;
	int retCode = 0;

	// SCOPE
	do
	{
		if (hClientSocket == NULL)
		{
			DEBUG_BREAK();
			retCode = 0;
			break;
		}

		recvDatas = (char*)malloc(BUFFER_SIZE);

		if (recvDatas == NULL)
		{
			DEBUG_BREAK();
			retCode = 0;
			break;
		}

		while (true)
		{
			// ����
			memset(recvDatas, 0, BUFFER_SIZE);
			const int recvLen = recv(hClientSocket, recvDatas, BUFFER_SIZE, 0);
			if (recvLen == -1) {
				DEBUG_BREAK();
				retCode = 0;
				break;
			}
			printf("> received: %s\n", recvDatas);

			// ���� ����
			if (strcmp(recvDatas, "quit") == 0) {
				retCode = 1;
				break;
			}
		}
	} while (false);
	// SCOPE END

	if (recvDatas != NULL)
	{
		free(recvDatas);
		recvDatas = NULL;
	}

	return retCode;
}


int main_()
{
	char host[] = "127.0.0.1";
	unsigned short port = 65456;

	WSADATA wsaData;
	SOCKET hClientSocket;
	SOCKADDR_IN servAddr;

	MyThread* clientThread = nullptr;
	char* sendDatas = NULL;

	// SCOPE
	do {
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			DEBUG_BREAK();
			break;
		}
		
		hClientSocket = socket(PF_INET, SOCK_STREAM, 0);
		if (hClientSocket == INVALID_SOCKET)
		{
			DEBUG_BREAK();
			break;
		}

		unsigned long hostIP = inet_addr(host);
		if (hostIP == INADDR_NONE)
		{
			DEBUG_BREAK();
			break;
		}

		memset(&servAddr, 0, sizeof(servAddr));
		servAddr.sin_family = AF_INET;
		servAddr.sin_addr.s_addr = hostIP;
		servAddr.sin_port = htons(port);

		try
		{
			if (connect(hClientSocket, (SOCKADDR*)&servAddr, sizeof(servAddr)) == SOCKET_ERROR)
			{
				throw std::runtime_error("> connect() failed and program terminated\n");
			}
		}
		catch (const std::exception& ex)
		{
			std::cerr << "> connect() failed by exception: \n" << ex.what();
			break;
		}

		clientThread = new MyThread(std::bind(recvHandler, hClientSocket));
		if (clientThread == nullptr)
		{
			DEBUG_BREAK();
			break;
		}
		clientThread->Start();
		

		sendDatas = (char*)malloc(BUFFER_SIZE);
		if (sendDatas == NULL)
		{
			DEBUG_BREAK();
			break;
		}

		bool isRunning = true;
		while (isRunning) {
			memset(sendDatas, 0, BUFFER_SIZE);
			printf("> ");
			fgets(sendDatas, BUFFER_SIZE, stdin);  // �Է� �ޱ� (���� ���� ����)

			// ���� ���� ����
			size_t sendLen = strlen(sendDatas);
			if (sendLen > 0 && sendDatas[sendLen - 1] == '\n') {
				sendDatas[sendLen - 1] = '\0';
				--sendLen;
			}

			// SendAll Start
			size_t accumulBytesSent = 0;
			while (accumulBytesSent < sendLen)
			{
				size_t bytesSent = send(hClientSocket, sendDatas + accumulBytesSent, sendLen - accumulBytesSent, 0);
				if (bytesSent == SOCKET_ERROR)
				{
					DEBUG_BREAK();
					isRunning = false;
					break;
				}
				accumulBytesSent += bytesSent;
			}
			// SendAll End

			
			// ���� ����
			if (strcmp(sendDatas, "quit") == 0) {
				isRunning = false;
				break;
			}
		}
	} while (false);
	// SCOPE END
	

	if (clientThread != nullptr)
	{
		clientThread->Join();
		delete clientThread;
		clientThread = nullptr;
	}

	if (sendDatas != NULL)
	{
		free(sendDatas);
		sendDatas = NULL;
	}
	
	if (hClientSocket != NULL)
	{
		closesocket(hClientSocket);
		hClientSocket = NULL;
	}

	WSACleanup();

	return 0;
}

int main()
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(167);
#endif  // _DEBUG

	printf("> echo - client is activated\n");
	main_();
	printf("> echo-client is de-activated\n");


#ifdef _DEBUG
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
	_CrtDumpMemoryLeaks();
#endif  // _DEBUG
}