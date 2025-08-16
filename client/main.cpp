#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

static void print_usage(void) {
    printf("echo-client:\n");
    printf("syntax : echo-client <ip> <port>\n");
    printf("sample : echo-client 192.168.10.2 1234\n");
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        print_usage();
        return 1;
    }
    const char* server_ip = argv[1];
    const char* server_port = argv[2];

    int sock;
    struct sockaddr_in server;
    char msg[1024], server_res[1024];

    /* 소켓 생성 (ipv4, TCP, protocol auto) */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("socket create error\n");
        return -1;
    }
    printf("socket created\n");

    /* ip, protocol, port 저장 */
    server.sin_addr.s_addr = inet_addr(server_ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(server_port));

    /* server와 연결 */
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("server connect error\n");
        return 1;
    }
    printf("connected\n");

    /* 무한 루프 */
    while (1) {
        scanf("%s", msg);

        if (send(sock, msg, strlen(msg), 0) < 0) {
            printf("send error\n");
            break;
        }

        int bytes_received = recv(sock, server_res, sizeof(server_res), 0);
        if (bytes_received == 0) {
            printf("server closed connection\n");
            break;
        } else if (bytes_received < 0) {
            printf("recv error\n");
            break;
        }

        server_res[bytes_received] = '\0'; // null terminate the response
        printf("%s\n", server_res);
    }

    close(sock);
    printf("connect close\n");
    return 0;
}
