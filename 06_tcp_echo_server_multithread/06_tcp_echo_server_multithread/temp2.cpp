//#include <iostream>
//#include <winsock2.h>
//#include <process.h>
//#include <map>
//
//#pragma comment(lib, "ws2_32.lib")
//#define BUFFER_SIZE 1024
//#define _WINSOCK_DEPRECATED_NO_WARNINGS
//
//class BaseRequestHandler {
//public:
//	virtual unsigned int Handle(SOCKET hClientSocket, const sockaddr_in& clientAddress) = 0;
//	virtual ~BaseRequestHandler() = default;
//};
//
//class MyTCPSocketHandler : public BaseRequestHandler {
//public:
//	MyTCPSocketHandler(int sizeBuffer) : sizeBuffer_(sizeBuffer) {
//		recvDatas_ = new char[sizeBuffer_];
//		if (!recvDatas_) {
//			throw std::runtime_error("Failed to allocate buffer");
//		}
//	}
//	~MyTCPSocketHandler() {
//		delete[] recvDatas_;
//	}
//
//	unsigned int Handle(SOCKET hClientSocket, const sockaddr_in& clientAddress) override {
//		printf("> client connected by IP address %s with Port number %u\n",
//			inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
//
//		while (true) {
//			memset(recvDatas_, 0, sizeBuffer_);
//			int recvLen = recv(hClientSocket, recvDatas_, sizeBuffer_, 0);
//			if (recvLen <= 0) {
//				break;
//			}
//
//			printf("> echoed: %s\n", recvDatas_);
//
//			// SendAll
//			int sent = 0;
//			while (sent < recvLen) {
//				int bytesSent = send(hClientSocket, recvDatas_ + sent, recvLen - sent, 0);
//				if (bytesSent == -1) {
//					return -1;
//				}
//				sent += bytesSent;
//			}
//
//			if (recvLen >= 4 && strncmp(recvDatas_, "quit", 4) == 0) {
//				break;
//			}
//		}
//
//		closesocket(hClientSocket);
//		return 0;
//	}
//
//private:
//	char* recvDatas_;
//	int sizeBuffer_;
//};
//
//class EchoServer {
//public:
//	EchoServer(const char* host, unsigned short port, BaseRequestHandler* handler)
//		: host_(host), port_(port), handler_(handler) {
//		if (WSAStartup(MAKEWORD(2, 2), &wsaData_) != 0) {
//			throw std::runtime_error("WSAStartup failed");
//		}
//	}
//
//	~EchoServer() {
//		CleanUp();
//		delete handler_;
//	}
//
//	void ServeForever() {
//		SOCKET serverSocket = socket(PF_INET, SOCK_STREAM, 0);
//		if (serverSocket == INVALID_SOCKET) {
//			throw std::runtime_error("socket() failed");
//		}
//
//		unsigned long hostIP = inet_addr(host_);
//		if (hostIP == INADDR_NONE) {
//			throw std::runtime_error("Invalid IP address");
//		}
//
//		SOCKADDR_IN serverAddress{};
//		serverAddress.sin_family = AF_INET;
//		serverAddress.sin_addr.s_addr = hostIP;
//		serverAddress.sin_port = htons(port_);
//
//		if (bind(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
//			closesocket(serverSocket);
//			throw std::runtime_error("bind() failed");
//		}
//
//		if (listen(serverSocket, 10) == SOCKET_ERROR) {
//			closesocket(serverSocket);
//			throw std::runtime_error("listen() failed");
//		}
//
//		printf("> echo - server is activated\n");
//
//		while (true) {
//			SOCKADDR_IN clientAddress{};
//			int addrSize = sizeof(clientAddress);
//			SOCKET clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddress, &addrSize);
//			if (clientSocket == INVALID_SOCKET) {
//				break;
//			}
//
//			ThreadArg* arg = new ThreadArg{ clientSocket, clientAddress, handler_ };
//			unsigned int threadId = 0;
//			HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0, ThreadFunc, arg, 0, &threadId);
//			if (hThread) {
//				threads_[threadId] = hThread;
//			}
//			else {
//				delete arg;
//			}
//		}
//
//		closesocket(serverSocket);
//	}
//
//private:
//	struct ThreadArg {
//		SOCKET socket;
//		sockaddr_in address;
//		BaseRequestHandler* handler;
//	};
//
//	static unsigned int __stdcall ThreadFunc(void* argPtr) {
//		ThreadArg* arg = static_cast<ThreadArg*>(argPtr);
//		if (!arg) {
//			return -1;
//		}
//
//		unsigned int result = arg->handler->Handle(arg->socket, arg->address);
//		delete arg;
//		_endthreadex(result);
//		return result;
//	}
//
//	void CleanUp() {
//		for (auto& [id, handle] : threads_) {
//			WaitForSingleObject(handle, INFINITE);
//			CloseHandle(handle);
//		}
//		threads_.clear();
//
//		WSACleanup();
//	}
//
//	const char* host_;
//	const unsigned short port_;
//	BaseRequestHandler* handler_;
//	WSADATA wsaData_;
//	std::map<unsigned int, HANDLE> threads_;
//};
//
//int main() {
//	try {
//		const char* host = "127.0.0.1";
//		unsigned short port = 65456;
//
//		BaseRequestHandler* handler = new MyTCPSocketHandler(BUFFER_SIZE);
//		EchoServer server(host, port, handler);
//		server.ServeForever();
//
//		printf("> echo - server terminated\n");
//	}
//	catch (const std::exception& ex) {
//		std::cerr << "[Exception] " << ex.what() << std::endl;
//	}
//
//	return 0;
//}