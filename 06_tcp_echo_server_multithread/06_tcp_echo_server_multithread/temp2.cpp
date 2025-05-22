#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <WinSock2.h>
#pragma comment(lib, "Ws2_32.lib")

// TCP Socket Handler
class MyTCPSocketHandler {
private:
    SOCKET clientSocket;

public:
    MyTCPSocketHandler(SOCKET clientSocket)
        : clientSocket(clientSocket) {
    }

    // 클라이언트 연결 처리 메서드
    void HandleClient() {
        char buffer[4096];
        int bytesRead;

        while (true) {
            bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead > 0) {
                // 에코 메시지 전송
                send(clientSocket, buffer, bytesRead, 0);
            }
            else {
                // 클라이언트 연결 종료
                closesocket(clientSocket);
                break;
            }
        }
    }
};

// 멀티쓰레드 에코 서버
template <typename SocketHandler>
class EchoServer {
private:
    const char* host;
    unsigned short port;
    SOCKET serverSocket;
    std::vector<std::thread> threads;
    std::mutex mtx;
    std::condition_variable cv;
    bool stopServer = false;

public:
    EchoServer(const char* host, unsigned short port)
        : host(host), port(port) {
        // Winsock 초기화
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("Failed to initialize Winsock");
        }

        // 서버 소켓 생성
        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET) {
            WSACleanup();
            throw std::runtime_error("Failed to create server socket");
        }

        // 서버 주소 설정
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(host);
        serverAddr.sin_port = htons(port);

        // 서버 바인딩
        if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(serverSocket);
            WSACleanup();
            throw std::runtime_error("Failed to bind server socket");
        }

        // 서버 시작
        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            closesocket(serverSocket);
            WSACleanup();
            throw std::runtime_error("Error listening on server socket");
        }
    }

    // 클라이언트 연결을 받고 처리하는 메서드
    void ServeForever() {
        while (!stopServer) {
            // 클라이언트 연결 수락
            SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "Failed to accept client connection" << std::endl;
                continue;
            }

            // 클라이언트 핸들러를 스레드 풀에서 실행
            std::unique_lock<std::mutex> lock(mtx);
            threads.emplace_back([this, clientSocket]() {
                SocketHandler handler(clientSocket);
                handler.HandleClient();
                });
        }
    }

    // 서버 종료
    void Stop() {
        stopServer = true;
        closesocket(serverSocket);
        WSACleanup();

        // 모든 스레드가 종료될 때까지 대기
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    ~EchoServer() {
        Stop();
    }
};

int main() {
    const char* host = "127.0.0.1";
    const unsigned short port = 65456;

    EchoServer<MyTCPSocketHandler>* echoServer = new EchoServer<MyTCPSocketHandler>(host, port);
    if (echoServer == nullptr) {
        std::cerr << "Failed to create EchoServer instance" << std::endl;
        return 1;
    }

    std::cout << "> echo - server is activated" << std::endl;
    try {
        echoServer->ServeForever();
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
    std::cout << "> echo - server is deactivated" << std::endl;

    delete echoServer;
    return 0;
}
