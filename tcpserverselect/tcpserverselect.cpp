#include <cstring>
#include <iostream>
#include <set>

#include <WinSock2.h>
#include <WS2tcpip.h>

// ws2_32.lib 를 링크한다.
#pragma comment(lib, "Ws2_32.lib")

static unsigned short SERVER_PORT = 27015;

int main()
{
    int r = 0;

    // Winsock 을 초기화한다.
    WSADATA wsaData;
    r = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (r != NO_ERROR) {
        std::cerr << "WSAStartup failed with error " << r << std::endl;
        return 1;
    }

    // TCP socket 을 만든다.
    SOCKET passiveSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (passiveSock == INVALID_SOCKET) {
        std::cerr << "socket failed with error " << WSAGetLastError() << std::endl;
        return 1;
    }

    // socket 을 특정 주소, 포트에 바인딩 한다.
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    r = bind(passiveSock, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (r == SOCKET_ERROR) {
        std::cerr << "bind failed with error " << WSAGetLastError() << std::endl;
        return 1;
    }

    // TCP 는 연결을 받는 passive socket 과 실제 통신을 할 수 있는 active socket 으로 구분된다.
    // passive socket 은 socket() 뒤에 listen() 을 호출함으로써 만들어진다.
    // active socket 은 passive socket 을 이용해 accept() 를 호출함으로써 만들어진다.
    r = listen(passiveSock, 10);
    if (r == SOCKET_ERROR) {
        std::cerr << "listen faijled with error " << WSAGetLastError() << std::endl;
        return 1;
    }

    // 끊어지지 않고 남아있는 active socket 들을 기억한다.
    std::set<int> allActiveSockets;

    std::cout << "Waiting for a connection" << std::endl;
    while (true) {
        // 읽기 이벤트를 위한 구조체 및 초기화
        fd_set rset;
        FD_ZERO(&rset);

        // 새 연결이 있는지를 확인해야되니 passive socket 을 읽기 체크 대상에 넣는다.
        FD_SET(passiveSock, &rset);
        int max = passiveSock;

        // 현재 active 소켓들을 읽기 체크 대상에 넣는다.
        for (int activeSock : allActiveSockets) {
            FD_SET(activeSock, &rset);
            if (activeSock > max) {
                max = activeSock;
            }
        }

        // select 를 호출한다.
        // select 는 우리가 지정한 소켓 중에 읽기 이벤트가 있는 것만 rset 에 남겨둔다.
        r = select(max + 1, &rset, NULL, NULL, NULL);
        if (r == SOCKET_ERROR) {
            std::cerr << "select failed " << WSAGetLastError() << std::endl;
            return 1;
        }

        // passive socket 이 읽기 가능이라는 말은 새 연결이 있다는 뜻이다.
        if (FD_ISSET(passiveSock, &rset)) {
            // passive socket 을 이용해 accept() 로 연결을 만든다.
            // 이미 새 연결이 있다고 했으니 accept() 는 blocking 되지 않는다.
            // 연결이 완료되고 만들어지는 소켓은 active socket 이다.
            struct sockaddr_in clientAddr;
            int clientAddrSize = sizeof(clientAddr);
            SOCKET activeSock = accept(passiveSock, (sockaddr*)&clientAddr, &clientAddrSize);
            if (activeSock == INVALID_SOCKET) {
                std::cerr << "accept failed with error " << WSAGetLastError() << std::endl;
                return 1;
            }

            // 다음 번에 active socket 에 대해서 읽기 이벤트가 있는지 확인하기 위해서 저장한다.
            allActiveSockets.insert(activeSock);
            std::cout << "New active socket " << activeSock << std::endl;
        }

        // allActiveSockets 의 loop 를 돌면서 거기서 값을 빼면 안되니
        // 닫힌 소켓은 별도로 저장했다가 나중에 뺀다.
        std::set<int> closedSockets;

        // active socket 들에 대해서 읽기 가능인지 확인한다.
        for (auto activeSock : allActiveSockets) {
            // 읽기 이벤트가 발생하지 않은 소켓이라면 무시한다.
            if (FD_ISSET(activeSock, &rset) == false) {
                continue;
            }

            // socket 으로부터 데이터를 받는다.
            std::cout << "[" << activeSock << "] Receiving stream" << std::endl;
            char buf[65536 * 2];
            r = recv(activeSock, buf, sizeof(buf), 0);
            if (r == SOCKET_ERROR) {
                std::cerr << "[" << activeSock << "] recv failed with error " << WSAGetLastError() << std::endl;
                closedSockets.insert(activeSock);
                continue;
            }

            // 소켓에서 0 byte 를 읽으면 소켓이 닫혔다는 뜻이다.
            if (r == 0) {
                std::cout << "[" << activeSock << "] socket closed" << std::endl;

                // 닫힌 소켓을 for 밖에서 제거하기 위해서 기억한다.
                closedSockets.insert(activeSock);

                // 이 특정 연결의 activeSock 을 닫는다.
                r = closesocket(activeSock);
                if (r == SOCKET_ERROR) {
                    std::cerr << "[" << activeSock << "] closesocket(acitve) failed with error " << WSAGetLastError() << std::endl;
                    closedSockets.insert(activeSock);
                    continue;
                }
            }
            else {
                std::cout << "[" << activeSock << "] Received " << r << " bytes" << std::endl;
            }
        }

        // 닫힌 소켓을 제외한다.
        for (auto closedSock : closedSockets) {
            allActiveSockets.erase(closedSock);
        }
    }

    // 연결을 기다리는 passive socket 을 닫는다.
    r = closesocket(passiveSock);
    if (r == SOCKET_ERROR) {
        std::cerr << "closesocket(passive) failed with error " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Winsock 을 정리한다.
    WSACleanup();
    return 0;
}
