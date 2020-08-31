#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <arpa/inet.h>
#include "../etc/config.hpp"
#include "../include/ev_epoll.h"
int front_tcp_init(int port, int max_connection);
int back_tcp_init(const char *ip_addr);
int tcp_accept(int epoll_fd, int fd);
int tcp_receive(int client_fd, void *buf, int64_t n);
int tcp_send(int sock_fd, char *buffer, int64_t length);
#endif