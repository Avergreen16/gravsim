#include <winsock2.h>
#include <iostream>
#include <array>
#include <vector>
#include <thread>

struct prng {
    double value;
    double factor;

    double operator() () {
        double temp = value * factor;
        value = temp - int(temp);
        return value;
    }
};

void play_rpss(SOCKET s) {
    prng rng = {double(rand() % 100) / 100, 43.5784987};
    bool quit_game = false;
    while(!quit_game) {
        int response = rng() * 3;
        std::array<int, 2> response_array = {response, 1};
        char player_choice;

        // 0 = rock, 1 = paper, 2 = scissors

        recv(s, &player_choice, 1, 0);

        switch(player_choice) {
            case 'q':
                quit_game = true;
                closesocket(s);

            case 'r':
                if(response == 0) {
                    response_array[1] = 0;
                } else if(response == 1) {
                    response_array[1] = -1;
                } else if(response == 2) {
                    response_array[1] = 1;
                }
                break;

            case 'p':
                if(response == 0) {
                    response_array[1] = 1;
                } else if(response == 1) {
                    response_array[1] = 0;
                } else if(response == 2) {
                    response_array[1] = -1;
                }
                break;

            case 's':
                if(response == 0) {
                    response_array[1] = -1;
                } else if(response == 1) {
                    response_array[1] = 1;
                } else if(response == 2) {
                    response_array[1] = 0;
                }
                break;
        }
        if(quit_game) break;

        send(s, (char*)&response_array, sizeof(response_array), 0);
    }
}

int main() {
    WSADATA wsadata;
    int wsaerr = WSAStartup(0x0202, &wsadata);
    if(wsaerr != 0) {
        std::cout << "WSAStartup error: " << wsadata.szSystemStatus << "\n";
        return 1;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server_socket == INVALID_SOCKET) {
        std::cout << "socket error: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    unsigned short int server_port = 16003;
    std::string server_ip = "127.0.0.1";

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(server_ip.c_str());
    address.sin_port = htons(server_port);

    if(bind(server_socket, (SOCKADDR*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cout << "bind error: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }
    std::vector<std::thread> thread_vector;

    for(int i = 0; i < 5; i++) {
        if(listen(server_socket, 20) == SOCKET_ERROR) {
            std::cout << "listen error: " << WSAGetLastError() << "\n";
            WSACleanup();
            return 1;
        }
        std::cout << "listening for connections...\n";

        SOCKET accept_socket = accept(server_socket, NULL, NULL);
        if(accept_socket == INVALID_SOCKET) {
            std::cout << "accept error: " << WSAGetLastError() << "\n";
        } else {
            std::cout << "connection accepted\n";
            thread_vector.push_back(std::thread(play_rpss, accept_socket));
        }
    }

    for(std::thread &t : thread_vector) {
        if(t.joinable()) t.join();
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}