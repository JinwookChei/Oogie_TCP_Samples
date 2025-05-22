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

    // Ŭ���̾�Ʈ ���� ó�� �޼���
    void HandleClient() {
        char buffer[4096];
        int bytesRead;

        while (true) {
            bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead > 0) {
                // ���� �޽��� ����
                send(clientSocket, buffer, bytesRead, 0);
            }
            else {
                // Ŭ���̾�Ʈ ���� ����
                closesocket(clientSocket);
                break;
            }
        }
    }
};

// ��Ƽ������ ���� ����
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
        // Winsock �ʱ�ȭ
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            throw std::runtime_error("Failed to initialize Winsock");
        }

        // ���� ���� ����
        serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET) {
            WSACleanup();
            throw std::runtime_error("Failed to create server socket");
        }

        // ���� �ּ� ����
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(host);
        serverAddr.sin_port = htons(port);

        // ���� ���ε�
        if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(serverSocket);
            WSACleanup();
            throw std::runtime_error("Failed to bind server socket");
        }

        // ���� ����
        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            closesocket(serverSocket);
            WSACleanup();
            throw std::runtime_error("Error listening on server socket");
        }
    }

    // Ŭ���̾�Ʈ ������ �ް� ó���ϴ� �޼���
    void ServeForever() {
        while (!stopServer) {
            // Ŭ���̾�Ʈ ���� ����
            SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "Failed to accept client connection" << std::endl;
                continue;
            }

            // Ŭ���̾�Ʈ �ڵ鷯�� ������ Ǯ���� ����
            std::unique_lock<std::mutex> lock(mtx);
            threads.emplace_back([this, clientSocket]() {
                SocketHandler handler(clientSocket);
                handler.HandleClient();
                });
        }
    }

    // ���� ����
    void Stop() {
        stopServer = true;
        closesocket(serverSocket);
        WSACleanup();

        // ��� �����尡 ����� ������ ���
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
