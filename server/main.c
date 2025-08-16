#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

static void print_usage(void) {
    printf("echo-server:\n");
    printf("syntax : echo-server <port> [-e[-b]]\n");
    printf("sample : echo-server 1234 -e -b\n");
}

int main(int argc, char* argv[]) {
    const char* port = argv[1];
    bool opt_echo = false;
    bool opt_broadcast = false;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-e") == 0) opt_echo = true;
        else if (strcmp(argv[i], "-b") == 0) opt_broadcast = true;
        else {
            print_usage();
            return 1;
        }
    }

    int server_fd, new_socket;
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

    /* 연결 요청 기다리기 */
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        return 1;
    }

    /* 연결 요청 수락 */
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    if (new_socket < 0) {
        perror("accept failed");
        close(server_fd);
        return 1;
    }

    /* 송수신 */
    while (1) {
        char buffer[1024];
        int bytes_read = recv(new_socket, buffer, sizeof(buffer), 0);

        if (bytes_read == 0) {
            printf("Client disconnected\n");
            break;
        } else if (bytes_read < 0) {
            perror("recv failed");
            break;
        }

        printf("%s\n", buffer);
    }

    close(new_socket);
    close(server_fd);

    return 0;
}
