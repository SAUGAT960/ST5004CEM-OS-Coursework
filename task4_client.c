
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER 1024

int main(int argc, char* argv[]) {
    if(argc < 3) {
        printf("Usage: %s <client_id> <command>\n", argv[0]);
        printf("Commands: AUTH:<user> | MSG:<text> | QUIT\n");
        return 1;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0) { perror("Socket failed"); return 1; }

    struct sockaddr_in addr = {AF_INET, htons(PORT)};
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if(connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Connect failed"); return 1;
    }

    printf("[CLIENT-%s] Connected to server\n", argv[1]);

    char* cmd = argv[2];
    write(fd, cmd, strlen(cmd));
    printf("[CLIENT-%s] Sent: %s\n", argv[1], cmd);

    char resp[BUFFER];
    int n = read(fd, resp, BUFFER-1);
    if(n > 0) {
        resp[n] = '\0';
        printf("[CLIENT-%s] Server response: %s\n", argv[1], resp);
    }

    close(fd);
    printf("[CLIENT-%s] Disconnected\n", argv[1]);
    return 0;
}
