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
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), '\0', 8);

    bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr));

    char buffer[1024];

    struct sockaddr_in client_addr;
    
    socklen_t client_len = sizeof(client_addr);

    int n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&client_addr, &client_len);

    buffer[n] = '\0';

    printf("msg from client: %s\n", buffer);

    char *response = "Hi from server";

    sendto( sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, (socklen_t)sizeof(client_addr));

    close(sockfd);
    
}