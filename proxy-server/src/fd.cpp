

#include "../include/fd.h"

std::unordered_map<int, int> fdtab; //用于存储前端和后端对应的fd对
extern int epfd;

void fdpaircreate(int newfd) //目前还是单线程使用不加锁
{
    //printf("newfd 为%d\n", newfd);
    int backfd = back_tcp_init(BACKEND_IP_ADDR_1); //目前单server，后续修改
    //printf("backfd 为%d\n", backfd);
    fdtab[newfd] = backfd;
    //printf("fdtab[newfd] 为%d\n", fdtab[newfd]);
    fdtab[backfd] = newfd;
    //printf("fdtab[backfd] 为%d\n", fdtab[backfd]);
    epoll_register(EPOLLIN | EPOLLET | EPOLLONESHOT, epfd, backfd);
}