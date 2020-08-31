//
// Created by Jiashuai on 2020/8/17.
//

#ifndef SERVER_HELPER_H
#define SERVER_HELPER_H

#include <cstdlib>
#include <cstdio>
#include <poll.h>
#include <fcntl.h>
#include <sys/epoll.h>



int epoll_register(int events_, int epoll_fd_, int fd_);

void set_unblock(int fd);


#endif //SERVER_HELPER_H
