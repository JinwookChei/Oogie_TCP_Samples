//#include <iostream>
//#include <stdio.h>
//#include <stdlib.h>
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <process.h>
//#include <Windows.h>
//#include <map>
//
//
//#pragma comment(lib, "ws2_32.lib")
//#define BUFFER_SIZE 1024
//
//#ifdef _DEBUG
//#define DEBUG_BREAK() __debugbreak()
//#else
//#define DEBUG_BREAK() ((void)0)
//#endif
//
//
//class BaseRequestHandler
//{
//public:
//	BaseRequestHandler()
//		: server_(nullptr),
//		hClientSocket_(INVALID_SOCKET),
//		clientAddress_()
//	{
//	}
//
//	virtual ~BaseRequestHandler()
//	{
//		if (hClientSocket_ != INVALID_SOCKET) {
//			closesocket(hClientSocket_);
//			hClientSocket_ = INVALID_SOCKET;
//		}
//	}
//
//	bool Init(void* server, SOCKET hClientSocket, SOCKADDR_IN clientAddress)
//	{
//		if (server == nullptr)
//		{
//			return false;
//		}
//		server_ = server;
//		hClientSocket_ = hClientSocket;
//		clientAddress_ = clientAddress;
//
//		return true;
//	}
//
//	virtual unsigned int Handle() = 0;
//
//	static unsigned __stdcall ThreadEntry(void* arg) {
//		BaseRequestHandler* requestHandler = static_cast<BaseRequestHandler*>(arg);
//		requestHandler->Handle();
//		requestHandler->Exit();
//		return 0;
//	}
//
//	void Exit()
//	{
//		if (server_ == nullptr)
//		{
//			DEBUG_BREAK();
//			return;
//		}
//
//		static_cast<EchoServer<BaseRequestHandler>*>(server_)->TerminateThread(this);
//	}
//
//protected:
//	void* server_;
//	SOCKET hClientSocket_;
//	SOCKADDR_IN clientAddress_;
//};
//
//class MyTCPSocketHandler : public BaseRequestHandler {
//public:
//	MyTCPSocketHandler() {
//		recvDatas_ = new char[BUFFER_SIZE];
//	}
//
//	~MyTCPSocketHandler() {
//		CleanUp();
//	}
//
//	unsigned int Handle() override {
//		printf("> client connected by IP address %s with Port number %u\n", inet_ntoa(clientAddress_.sin_addr), ntohs(clientAddress_.sin_port));
//
//		while (true)
//		{
//			if (recvDatas_ == nullptr)
//			{
//				__debugbreak();
//				CleanUp();
//				break;
//			}
//			memset(recvDatas_, 0, BUFFER_SIZE);
//			const int recvLen = recv(hClientSocket_, recvDatas_, BUFFER_SIZE, 0);
//			if (recvLen <= 0)
//			{
//				CleanUp();
//				break;
//			}
//
//			printf("> echoed: %s\n", recvDatas_);
//
//			// SendAll Start
//			int accumulBytesSent = 0;
//			while (accumulBytesSent < recvLen)
//			{
//				int bytesSent = send(hClientSocket_, recvDatas_ + accumulBytesSent, recvLen - accumulBytesSent, 0);
//				if (bytesSent == -1)
//				{
//					__debugbreak();
//					break;
//				}
//				accumulBytesSent += bytesSent;
//			}
//			// SendAll End
//
//			if (strcmp(recvDatas_, "quit") == 0)
//			{
//				break;
//			}
//		}
//		return 0;
//	}
//
//	void CleanUp()
//	{
//		if (recvDatas_ != nullptr)
//		{
//			delete[] recvDatas_;
//			recvDatas_ = nullptr;
//		}
//	}
//private:
//	char* recvDatas_;
//};
//
//template<typename HandlerType>
//class EchoServer
//{
//	friend BaseRequestHandler;
//public:
//	EchoServer(const char* host, unsigned short port)
//		: host_(host),
//		port_(port),
//		hServerSocket_(INVALID_SOCKET),
//		serverAddress_(),
//		clientCount_(0),
//		hMutex_(NULL)
//	{
//		if (WSAStartup(MAKEWORD(2, 2), &wsaData_) != 0)
//		{
//			DEBUG_BREAK();
//			return;
//		}
//
//		hMutex_ = CreateMutex(NULL, FALSE, NULL);
//		if (hMutex_ == NULL)
//		{
//			DEBUG_BREAK();
//			return;
//		}
//	}
//	~EchoServer()
//	{
//		WaitForSingleObject(hMutex_, INFINITE);
//		for (auto& entry : threads_) {
//			TerminateThread(entry.first);
//		}
//		ReleaseMutex(hMutex_);
//		CleanUp();
//	}
//
//	void ServeForever()
//	{
//		hServerSocket_ = socket(PF_INET, SOCK_STREAM, 0);
//		if (hServerSocket_ == INVALID_SOCKET)
//		{
//			DEBUG_BREAK();
//			throw std::runtime_error("> socket() failed");
//			CleanUp();
//			return;
//		}
//
//		unsigned long hostIP = inet_addr(host_);
//		if (hostIP == INADDR_NONE)
//		{
//			DEBUG_BREAK();
//			throw std::runtime_error("> Invalid IP address");
//			CleanUp();
//			return;
//		}
//
//		memset(&serverAddress_, 0, sizeof(serverAddress_));
//		serverAddress_.sin_family = AF_INET;
//		serverAddress_.sin_addr.s_addr = hostIP;
//		serverAddress_.sin_port = htons(port_);
//
//		if (bind(hServerSocket_, (SOCKADDR*)&serverAddress_, sizeof(serverAddress_)) == SOCKET_ERROR)
//		{
//			DEBUG_BREAK();
//			throw std::runtime_error("> bind() failed");
//			CleanUp();
//			return;
//		}
//
//		if (listen(hServerSocket_, 10) == SOCKET_ERROR)
//		{
//			DEBUG_BREAK();
//			throw std::runtime_error("> listen() failed");
//			CleanUp();
//			return;
//		}
//
//		while (true)
//		{
//			sockaddr_in clientAddress;
//			int sizeClientAddress = sizeof(clientAddress);
//			SOCKET clientSocket = accept(hServerSocket_, (SOCKADDR*)&clientAddress, &sizeClientAddress);
//			if (clientSocket == INVALID_SOCKET)
//			{
//				closesocket(clientSocket);
//				DEBUG_BREAK();
//				printf("> accept() failed\n");
//				continue;
//			}
//
//			BaseRequestHandler* requestHandler = new HandlerType;
//			if (requestHandler == nullptr)
//			{
//				DEBUG_BREAK();
//				printf("> requestHandler create failed\n");
//				continue;
//			}
//
//			if (requestHandler->Init(this, clientSocket, clientAddress) == false)
//			{
//				DEBUG_BREAK();
//				printf("> requestHandler Init failed\n");
//				continue;
//			}
//
//			HANDLE threadHandle = INVALID_HANDLE_VALUE;
//			threadHandle = (HANDLE)_beginthreadex(NULL, 0, &BaseRequestHandler::ThreadEntry, requestHandler, 0, NULL);
//			if (threadHandle == 0) {
//				DEBUG_BREAK();
//				printf("Failed to create thread\n");
//				delete requestHandler;
//				continue;
//			}
//			else
//			{
//				// MUTEX
//				threads_.insert(std::pair<BaseRequestHandler*, HANDLE>(requestHandler, threadHandle));
//				++clientCount_;
//				//
//			}
//		}
//	}
//
//	void TerminateThread(BaseRequestHandler* baseHandler)
//	{
//		// MUTEX 막아야함.
//		auto iter = threads_.find(baseHandler);
//		if (iter == threads_.end())
//		{
//			DEBUG_BREAK();
//		}
//		else
//		{
//			HANDLE hTherad = iter->second;
//			if (hTherad != nullptr)
//			{
//				CloseHandle(hTherad);
//			}
//			else
//			{
//				DEBUG_BREAK();
//			}
//		}
//
//		size_t removed = threads_.erase(baseHandler);
//		if (removed == 0)
//		{
//			DEBUG_BREAK();
//		}
//
//		--clientCount_;
//		delete baseHandler;
//		return;
//	}
//
//	void CleanUp()
//	{
//		if (hMutex_ != NULL)
//		{
//			CloseHandle(hMutex_);
//			hMutex_ = NULL;
//		}
//
//		if (hServerSocket_ != INVALID_SOCKET)
//		{
//			closesocket(hServerSocket_);
//			hServerSocket_ = INVALID_SOCKET;
//		}
//		WSACleanup();
//	}
//
//private:
//	const char* host_;
//	const unsigned short port_;
//	WSADATA wsaData_;
//	SOCKET hServerSocket_;
//	SOCKADDR_IN serverAddress_;
//
//	// 공유자원 사용 주의!
//	// TODO : Client CleanUP;
//	std::map<BaseRequestHandler*, HANDLE> threads_;
//	int clientCount_;
//	HANDLE hMutex_;
//};
//
//int main()
//{
//	const char* host = "127.0.0.1";
//	const unsigned short port = 65456;
//
//	EchoServer<MyTCPSocketHandler>* echoServer = new EchoServer<MyTCPSocketHandler>(host, port);
//	if (echoServer == nullptr)
//	{
//		DEBUG_BREAK();
//		return 0;
//	}
//
//	printf("> echo - server is activated\n");
//	try {
//		echoServer->ServeForever();
//	}
//	catch (const std::exception& ex) {
//		std::cerr << "Exception: " << ex.what() << std::endl;
//	}
//	printf("> echo - server is de-activated\n");
//
//	delete echoServer;
//	return 0;
//}


// 같은 include, define은 그대로 유지


#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
#include <Windows.h>
#include <map>
#include <mutex>

#pragma comment(lib, "ws2_32.lib")
#define BUFFER_SIZE 1024

#ifdef _DEBUG
#define DEBUG_BREAK() __debugbreak()
#else
#define DEBUG_BREAK() ((void)0)
#endif

std::mutex g_threadMutex;

class BaseRequestHandler {
public:
    BaseRequestHandler()
        : server_(nullptr), hClientSocket_(INVALID_SOCKET) {
    }

    virtual ~BaseRequestHandler() {
        if (hClientSocket_ != INVALID_SOCKET) {
            closesocket(hClientSocket_);
            hClientSocket_ = INVALID_SOCKET;
        }
    }

    bool Init(void* server, SOCKET hClientSocket, SOCKADDR_IN clientAddress) {
        if (server == nullptr) return false;
        server_ = server;
        hClientSocket_ = hClientSocket;
        clientAddress_ = clientAddress;
        return true;
    }

    virtual unsigned int Handle() = 0;

    static unsigned __stdcall ThreadEntry(void* arg) {
        BaseRequestHandler* requestHandler = static_cast<BaseRequestHandler*>(arg);
        requestHandler->Handle();
        requestHandler->Exit();
        return 0;
    }

    void Exit() {
        if (server_ == nullptr) return;
        static_cast<EchoServer<BaseRequestHandler>*>(server_)->JoinThread(this);
    }

protected:
    void* server_;
    SOCKET hClientSocket_;
    SOCKADDR_IN clientAddress_;
};

class MyTCPSocketHandler : public BaseRequestHandler {
public:
    MyTCPSocketHandler() {
        recvDatas_ = new char[BUFFER_SIZE];
    }

    ~MyTCPSocketHandler() {
        CleanUp();
    }

    unsigned int Handle() override {
        printf("> client connected from %s:%d\n",
            inet_ntoa(clientAddress_.sin_addr), ntohs(clientAddress_.sin_port));

        while (true) {
            memset(recvDatas_, 0, BUFFER_SIZE);
            int recvLen = recv(hClientSocket_, recvDatas_, BUFFER_SIZE, 0);
            if (recvLen <= 0) break;

            printf("> echoed: %s\n", recvDatas_);

            int totalSent = 0;
            while (totalSent < recvLen) {
                int sent = send(hClientSocket_, recvDatas_ + totalSent, recvLen - totalSent, 0);
                if (sent <= 0) break;
                totalSent += sent;
            }

            if (strcmp(recvDatas_, "quit") == 0) {
                break;
            }
        }
        return 0;
    }

    void CleanUp() {
        if (recvDatas_) {
            delete[] recvDatas_;
            recvDatas_ = nullptr;
        }
    }

private:
    char* recvDatas_;
};

template<typename HandlerType>
class EchoServer {
    friend class BaseRequestHandler;

public:
    EchoServer(const char* host, unsigned short port)
        : host_(host), port_(port), hServerSocket_(INVALID_SOCKET) {
        WSAStartup(MAKEWORD(2, 2), &wsaData_);
    }

    ~EchoServer() {
        for (auto& pair : threads_) {
            WaitForSingleObject(pair.second, INFINITE);
            CloseHandle(pair.second);
            delete pair.first;
        }
        threads_.clear();
        closesocket(hServerSocket_);
        WSACleanup();
    }

    void ServeForever() {
        hServerSocket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (hServerSocket_ == INVALID_SOCKET) throw std::runtime_error("socket failed");

        sockaddr_in serverAddr = {};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(host_);
        serverAddr.sin_port = htons(port_);

        if (bind(hServerSocket_, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
            throw std::runtime_error("bind failed");

        if (listen(hServerSocket_, SOMAXCONN) == SOCKET_ERROR)
            throw std::runtime_error("listen failed");

        while (true) {
            sockaddr_in clientAddr;
            int clientSize = sizeof(clientAddr);
            SOCKET clientSock = accept(hServerSocket_, (SOCKADDR*)&clientAddr, &clientSize);
            if (clientSock == INVALID_SOCKET) continue;

            HandlerType* handler = new HandlerType();
            handler->Init(this, clientSock, clientAddr);

            HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, &BaseRequestHandler::ThreadEntry, handler, 0, NULL);
            if (hThread == 0) {
                delete handler;
                continue;
            }

            std::lock_guard<std::mutex> lock(g_threadMutex);
            threads_[handler] = hThread;
        }
    }

    void JoinThread(BaseRequestHandler* handler) {
        std::lock_guard<std::mutex> lock(g_threadMutex);

        auto it = threads_.find(handler);
        if (it != threads_.end()) {
            WaitForSingleObject(it->second, INFINITE);
            CloseHandle(it->second);
            delete handler;
            threads_.erase(it);
        }
    }

private:
    const char* host_;
    unsigned short port_;
    WSADATA wsaData_;
    SOCKET hServerSocket_;
    std::map<BaseRequestHandler*, HANDLE> threads_;
};

int main() {
    const char* host = "127.0.0.1";
    const unsigned short port = 65456;

    EchoServer<MyTCPSocketHandler>* echoServer = new EchoServer<MyTCPSocketHandler>(host, port);
    if (!echoServer) {
        DEBUG_BREAK();
        return 1;
    }

    printf("> echo - server is activated\n");

    try {
        echoServer->ServeForever();
    }
    catch (const std::exception& e) {
        std::cerr << "Server exception: " << e.what() << std::endl;
    }

    printf("> echo - server is deactivated\n");
    delete echoServer;
    return 0;
}
