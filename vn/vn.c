#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define ON_PORT_NO 5000
#define PERIOD 10

int udp_fd, tcp_fd;
char label = '?';

int n_count;
char n_label[26];
uint32_t n_cost[26];
struct sockaddr_in n_addr[26];

uint32_t seq_no;
uint32_t seen[26];
time_t heard[26];
uint32_t graph[26][26];

void flood(uint8_t *lsp, int len) {
    for (int k = 0; k < 2; k++)
        for (int i = 0; i < n_count; i++)
            sendto(udp_fd, lsp, len, 0, (struct sockaddr *)&n_addr[i], sizeof(n_addr[i]));
}

void save_lsp(uint8_t *lsp) {
    int o = lsp[0] - 'A';
    memset(graph[o], 0, sizeof(graph[o]));
    for (int i = 0; i < lsp[5]; i++) {
        uint32_t c;
        memcpy(&c, lsp + 7 + i * 5, 4);
        graph[o][lsp[6 + i * 5] - 'A'] = ntohl(c);
    }
}

void send_lsp() {
    uint8_t lsp[136];
    seq_no++;
    uint32_t s = htonl(seq_no);
    lsp[0] = label;
    memcpy(lsp + 1, &s, 4);
    lsp[5] = n_count;
    for (int i = 0; i < n_count; i++) {
        uint32_t c = htonl(n_cost[i]);
        lsp[6 + i * 5] = n_label[i];
        memcpy(lsp + 7 + i * 5, &c, 4);
    }
    seen[label - 'A'] = seq_no;
    save_lsp(lsp);
    flood(lsp, 6 + n_count * 5);
}

void recv_lsp() {
    uint8_t lsp[136];
    int len = recvfrom(udp_fd, lsp, sizeof(lsp), 0, NULL, NULL);
    uint32_t s;
    memcpy(&s, lsp + 1, 4);
    s = ntohl(s);
    int o = lsp[0] - 'A';
    if (s <= seen[o])
        return;
    seen[o] = s;
    heard[o] = time(NULL);
    save_lsp(lsp);
    flood(lsp, len);
}

void recv_link_state() {
    uint8_t count;
    uint8_t buf[26 * 11];
    if (recv(tcp_fd, &count, 1, 0) <= 0) {
        printf("ON closed the connection\n");
        exit(0);
    }
    recv(tcp_fd, buf, count * 11, MSG_WAITALL);

    n_count = 0;
    for (int i = 0; i < count; i++) {
        uint8_t *t = buf + i * 11;
        uint32_t c;
        memcpy(&c, t + 7, 4);
        c = ntohl(c);
        if (c == 0) {
            label = t[0];
            continue;
        }
        memset(&n_addr[n_count], 0, sizeof(n_addr[n_count]));
        n_addr[n_count].sin_family = AF_INET;
        memcpy(&n_addr[n_count].sin_addr, t + 1, 4);
        memcpy(&n_addr[n_count].sin_port, t + 5, 2);
        n_label[n_count] = t[0];
        n_cost[n_count] = c;
        n_count++;
    }

    printf("%c=0", label);
    for (int i = 0; i < n_count; i++)
        printf(",%c=%u", n_label[i], n_cost[i]);
    printf("\n");
    fflush(stdout);

    send_lsp();
}

void forget_old() {
    for (int i = 0; i < 26; i++) {
        if (i != label - 'A' && seen[i] && time(NULL) - heard[i] > 3 * PERIOD) {
            seen[i] = 0;
            memset(graph[i], 0, sizeof(graph[i]));
        }
    }
}

void print_graph() {
    int n = 0;
    for (int i = 0; i < 26; i++)
        if (seen[i])
            n = i + 1;
    printf("graph at %c:\n", label);
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < i; j++)
            printf("\t");
        for (int j = i + 1; j < n; j++) {
            int c = graph[i][j] ? graph[i][j] : graph[j][i];
            if (c == 0)
                c = -1;
            printf(j == i + 1 ? "%d" : "\t%d", c);
        }
        printf("\n");
    }
    printf("end\n");
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("usage: ./vn <my_ip> <my_udp_port> <on_ip>\n");
        return 1;
    }

    struct sockaddr_in my_addr, on_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    memset(&on_addr, 0, sizeof(on_addr));
    my_addr.sin_family = on_addr.sin_family = AF_INET;
    inet_aton(argv[1], &my_addr.sin_addr);
    my_addr.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[3], &on_addr.sin_addr);
    on_addr.sin_port = htons(ON_PORT_NO);

    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (bind(udp_fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0) {
        perror("bind");
        return 1;
    }

    tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(tcp_fd, (struct sockaddr *)&on_addr, sizeof(on_addr)) < 0) {
        perror("connect");
        return 1;
    }

    uint8_t connect_msg[6];
    memcpy(connect_msg, &my_addr.sin_addr, 4);
    memcpy(connect_msg + 4, &my_addr.sin_port, 2);
    send(tcp_fd, connect_msg, 6, 0);

    time_t last = time(NULL);
    while (1) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(tcp_fd, &fds);
        FD_SET(udp_fd, &fds);
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int maxfd = tcp_fd > udp_fd ? tcp_fd : udp_fd;
        select(maxfd + 1, &fds, NULL, NULL, &tv);

        if (FD_ISSET(tcp_fd, &fds))
            recv_link_state();
        if (FD_ISSET(udp_fd, &fds))
            recv_lsp();
        if (label != '?' && time(NULL) - last >= PERIOD) {
            last = time(NULL);
            send_lsp();
            forget_old();
            print_graph();
        }
    }
    return 0;
}
