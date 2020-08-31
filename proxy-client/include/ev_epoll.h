#ifndef EV_EPOLL_H
#define EV_EPOLL_H

#include <stdlib.h>
#include <cstdlib>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <unordered_map>
#include <errno.h>
#include "../etc/config.hpp"
#include "../include/protocol.h"
#include "../include/fd.h"
#include "../include/worker.h"

//#include <poll.h>

int epoll_register(int events_, int epoll_fd_, int fd_);
int epoll_init(void);
//static void _do_poll(int pfd, int ffd, int bfd);
void _do_poll(int pfd);
void reset_oneshot(int epollfd, int fd);

struct Clientmsg
{
    char jobname[64];
    char filename[256];
    Clientmsg(char *job = nullptr, char *file = nullptr)
    {
        if (job)
            strcpy(jobname, job);
        if (file)
            strcpy(filename, file);
    }
};

#endif