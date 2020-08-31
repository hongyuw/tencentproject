#ifndef FD_H
#define FD_H

#include <unordered_map>
#include "../include/protocol.h"

void fdpaircreate(int newfd);

#define FD_EV_CLIENT 0
#define FD_EV_SERVER 1

struct fdtab
{
    int pair_fd;
    int owner;
};

#endif