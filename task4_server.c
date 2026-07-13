
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define PORT 8080
#define BUFFER 1024
#define MAX_CLIENTS 5

int auth_users = 0;

void handle_client(int cfd) {
    char buf[BUFFER];
    int n = read(cfd, buf, BUFFER-1);
    if(n > 0) {
        buf[n] = '\0';
        printf("[SERVER] Received: %s\n", buf);

        if(strncmp(buf, "AUTH:", 5) == 0) {
            char* user = buf + 5;
            printf("[SERVER] Authenticating user: %s\n", user);
            auth_users++;
            write(cfd, "AUTH_OK\n", 8);
            printf("[SERVER] Auth success. Total authenticated: %d\n", auth_users);
        }
        else if(strncmp(buf, "MSG:", 4) == 0) {
            if(auth_users > 0) {
                printf("[SERVER] Message received: %s\n", buf+4);
                write(cfd, "MSG_RECV\n", 9);
            } else {
                write(cfd, "ERR_NO_AUTH\n", 12);
            }
        }
        else if(strncmp(buf, "QUIT", 4) == 0) {
            write(cfd, "DISCONNECT\n", 11);
            printf("[SERVER] Client disconnected\n");
        }
        else {
            write(cfd, "ERR_UNKNOWN\n", 12);
            printf("[SERVER] Unknown command\n");
        }
    }
    close(cfd);
}

int main() {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sfd < 0) { perror("Socket failed"); return 1; }

    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {AF_INET, htons(PORT), INADDR_ANY};
    if(bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed"); return 1;
    }

    if(listen(sfd, MAX_CLIENTS) < 0) {
        perror("Listen failed"); return 1;
    }

    printf("============================================================\n");
    printf("  ST5004CEM - Task 4: Server Running on Port %d\n", PORT);
    printf("  Waiting for connections...\n");
    printf("============================================================\n\n");

    int client_count = 0;
    while(client_count < 3) {
        int cfd = accept(sfd, NULL, NULL);
        if(cfd < 0) continue;

        client_count++;
        printf("[SERVER] Client #%d connected (fd=%d)\n", client_count, cfd);

        pid_t pid = fork();
        if(pid == 0) {
            close(sfd);
            handle_client(cfd);
            exit(0);
        }
        close(cfd);
    }

    printf("\n[SERVER] Handled %d clients. Shutting down.\n", client_count);
    close(sfd);
    return 0;
}
