#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {

    struct sockaddr_in my_addr = {0}, on_addr = {0};
    my_addr.sin_family = on_addr.sin_family = AF_INET;
    inet_aton(argv[1], &my_addr.sin_addr);
    my_addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[3], &on_addr.sin_addr);
    on_addr.sin_port = htons(ON_PORT);
    
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);

}