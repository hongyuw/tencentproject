

//#include <poll.h>

#include "../include/ev_epoll.h"

extern int ffd;

extern std::unordered_map<int, int> fdtab;

int epoll_init(void)
{
    int epoll_fd;
    if ((epoll_fd = epoll_create(MAX_EVENTS)) < 0)
    {
        perror("epoll create failed\n");
        return 1;
    }
    return epoll_fd;
}
// 注册 server_fd 到 epoll，这里不能使用 ONESHOT 否则会丢失客户端数据?
int epoll_register(int events_, int epoll_fd_, int fd_)
{

    struct epoll_event event
    {
    };
    event.data.fd = fd_;
    event.events = events_;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd_, &event))
    {
        perror("epoll add failed in ctl\n");
        exit(1);
    }
    return 0;
}

void reset_oneshot(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

void _do_poll(int pfd)
{
    int count;
    int status;
    struct epoll_event events[MAX_EVENTS];
    // epoll_wait [错误]返回 -1 ，[超时]返回 0 ，[正常]返回获取到的事件数量
    do
    {
        status = epoll_wait(pfd, events, MAX_EVENTS, -1);
        // printf("epoll执行完成\n");
        //printf("struct %d", sizeof(Clientmsg));
        if (status)
        {
            break;
        }
        if (status < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                perror("epoll wait error\n");
                return;
            }
        }
    } while (1);
    /* process polled events */
    for (count = 0; count < status; count++)
    {
        auto fd = (int)events[count].data.fd;
        auto ev = (uint32_t)events[count].events;
        if ((ev & EPOLLERR) ||
            (ev & EPOLLHUP) ||
            !(ev & EPOLLIN))
        {
            perror("epoll event error\n");
            close(events[count].data.fd);
            continue;
        }
        /*如果是多线程的epoll要加锁*/
        else if ((fd == ffd) && (ev & EPOLLIN)) //前端请求连接
        {
            int cfd = tcp_accept(pfd, fd);
            // printf("前端建立连接");

            if (!fdtab.count(cfd)) //当前不存在此fd
            {
                printf("fd不存在");
                fdpaircreate(cfd);
            }
            else
            {
                printf("fd存在并删除");
                fdtab.erase(fdtab[cfd]); //删除之前与此fd配对的fd的键
                fdpaircreate(cfd);
            }

            /*前端连接建立以后就可以connect后端建立session，从而创建fd的键值对，暂时先创建两对*/
        }

        else if (ev & EPOLLIN) //前端和后端数据都可以
        {
            // printf("收到数据");
            // printf("收到数据的fd 为%d\n", fd);
            if (fdtab.count(fd)) //有可用的fd键值对前端一定可发数据吗，用select判断
            {

                //   printf("有可用连接接收数据");

                worker_put(fd);
            }    /*fd键值对已存在则向value值的fd发送数据*/
            else //没有键值对并且不是accept，直接丢弃（fd异常的时候处理fdtab,不在这里处理，希望在别的地方处理好）
            {
            }

            // ret = tcp_send(bfd, buffer, sizeof(buffer));
        }
        else if (ev & EPOLLIN && ev & EPOLLRDHUP)
        {
            printf("对端关闭连接");
        }
    }
}