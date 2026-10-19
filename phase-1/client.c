#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8000);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(&(server_addr.sin_zero), '\0', 8);

    char *message = "Hi from client";

    sendto(sockfd, message, strlen(message),0, (struct sockaddr *)&server_addr, (socklen_t)sizeof(server_addr));

    char buffer[1024];

    int n = recvfrom (sockfd, buffer, sizeof(buffer) - 1, 0, NULL, NULL);

    buffer[n] = '\0';

    printf("msg from server: %s\n", buffer);

    close(sockfd);
}