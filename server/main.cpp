#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>

static void print_usage(void) {
    printf("echo-server:\n");
    printf("syntax : echo-server <port> [-e[-b]]\n");
    printf("sample : echo-server 1234 -e -b\n");
}

// 전역 변수
static bool opt_echo = false;
static bool opt_broadcast = false;
static std::vector<int> client_sockets;
static std::mutex clients_mutex;

void handleClient(int client_socket);

int main(int argc, char* argv[]) {
    const char* port = argv[1];

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0) opt_echo = true;
        else if (strcmp(argv[i], "-b") == 0) opt_broadcast = true;
        else {
            print_usage();
            return 1;
        }
    }

    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    /* 소켓 생성 (ipv4, tcp, protocol auto) */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket creation failed");
        return 1;
    }

    /* 소켓에 주소 할당 */
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = AF_INET;
    address.sin_port = htons(atoi(port));
    
    if (bind(server_fd, (struct sockaddr *)&address, addrlen) < 0) {
        perror("bind failed");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        return 1;
    }
    printf("Server listening on port %s\n", port);
    
    std::vector<std::thread> client_threads;
    while (1) {
        int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("accept failed");
            continue;
        }
        printf("New client connected: %d\n", new_socket);
        
        // 클라이언트 소켓을 벡터에 추가
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            client_sockets.push_back(new_socket);
        }
        
        // 새 스레드에서 클라이언트 처리
        client_threads.emplace_back(handleClient, new_socket);
        client_threads.back().detach();
    }

    close(server_fd);
    return 0;
}

void broadcastMessage(const char* message, int message_len, int sender_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (auto it = client_sockets.begin(); it != client_sockets.end(); ) {
        int client_sock = *it;
        if (send(client_sock, message, message_len, 0) < 0) {
            printf("Client %d disconnected during broadcast\n", client_sock);
            close(client_sock);
            it = client_sockets.erase(it);
        } else {
            ++it;
        }
    }
}

void handleClient(int client_socket) {
    char buffer[1024];

    while (1) {
        int bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read == 0) {
            printf("Client disconnected\n");
            break;
        } else if (bytes_read < 0) {
            perror("recv failed");
            break;
        }
        
        buffer[bytes_read] = '\0';
        printf("%s\n", buffer);
        
        if (opt_echo && opt_broadcast) { // 모든 클라이언트에게 브로드캐스트
                broadcastMessage(buffer, bytes_read, client_socket);
        } else if (opt_echo) { // 해당 클라이언트에게만 에코
            if (send(client_socket, buffer, bytes_read, 0) < 0) {
                perror("send failed");
                break;
            }
        }
    }

    // 클라이언트가 연결 해제될 때 벡터에서 제거
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        client_sockets.erase(
            std::remove(client_sockets.begin(), client_sockets.end(), client_socket),
            client_sockets.end()
        );
    }
    
    close(client_socket);
}
