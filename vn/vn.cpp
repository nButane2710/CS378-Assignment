#include <iostream>
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]) {
    std::string ON_IP; std::cin >> ON_IP;
    int ON_PORT; std::cin >> ON_PORT; // ON_PORT == 5000

    std::string MY_IP; std::cin >> MY_IP;
    int MY_PORT; std::cin >> MY_PORT;
    std::cout << "done: taking input" << '\n';

    // open own udp port
    int sockfd_udp;
    sockfd_udp = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd_udp == -1){
        std::cerr << "error: upd socket open" << '\n';
        exit(1);
    }
    std::cout << "done: open own udp socket" << '\n';

    // bind own udp port
    struct sockaddr_in my_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(MY_PORT);
    my_addr.sin_addr.s_addr = inet_addr(MY_IP.c_str()); // c_str() to convert string to char*
    memset(&(my_addr.sin_zero), '\0', 8);
    if (bind(sockfd_udp, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1){
        std::cerr << "error: udp socket bind" << '\n';
        exit(1);
    }
    std::cout << "done: bind own udp port" << '\n';

    // open tcp port with on
    int sockfd_tcp;
    sockfd_tcp = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd_tcp == -1){
        std::cerr << "error: tcp socket open" << '\n';
        exit(1);
    }
    std::cout << "done: open tcp socket with on" << '\n';

    // connect to on
    struct sockaddr_in on_addr;
    on_addr.sin_family = AF_INET;
    on_addr.sin_port = htons(ON_PORT);
    on_addr.sin_addr.s_addr = inet_addr(DEST_IP.c_str());
    memset(&(on_addr.sin_zero), '\0', 8);
    if (connect(sockfd_tcp, (struct sockaddr *)&on_addr, sizeof(struct sockaddr)) == -1){
        std::cerr << "error: connect to on" << '\n';
        exit(1);
    }
    std::cout << "dont: tcp connect with on" << '\n';

    // send connect message to on
    char* connect_msg = "CONNECT";
    if (send(sockfd_tcp, connect_msg, strlen(connect_msg), 0) == -1){
        std::cerr << "error: send connect message to on" << '\n';
        exit(1);
    }
    cout << "done: send connect message to on" << '\n';

    // main loop
    while(1){
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd_tcp, &readfds);
        FD_SET(sockfd_udp, &readfds);

        int maxfd = (sockfd_tcp > sockfd_udp? sockfd_tcp : sockfd_udp) + 1;
        select(maxfd, &readfds, NULL, NULL, NULL); // sleep until one of them has data
        // also, removes all sockets which dont have data!

        if (FD_ISSET(sockfd_tcp, &readfds)){
            // message from ON
            recv(sockfd_tcp, ...);
            // print link state, send lsp to neighbours
        }
        if (FD_ISSET(sockfd_udp, &readfds)){
            // LSP from another VN
            recvfrom(sockfd_udp, ...);
            // update graph, forward if new
        }
    }
}