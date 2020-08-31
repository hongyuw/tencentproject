#include "../include/protocol.h"
#define __NR_gettid 186
extern std::unordered_map<int, int> fdtab;

int front_tcp_init(int port, int max_connection)
{
    int fd;
    // 创建 socket
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket error");
        return -1;
    }
    int reuse = 1;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    struct sockaddr_in server_sockaddr;               //socket结构体，#include <netinet/in.h>
    bzero(&server_sockaddr, sizeof(server_sockaddr)); //#include <string.h>,为什么需要这一步
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 本地任意ip
    server_sockaddr.sin_port = htons(port);
    // 绑定端口
    if (bind(fd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) == -1)
    {
        perror("bind error");
        return -1;
    }

    // 监听
    if (listen(fd, max_connection) < 0)
    {
        perror("listen error\n");
        return -1;
    }

    int err = -1;     /* 返回值 */
    int snd_size = 0; /* 发送缓冲区大小 */
    int rcv_size = 0; /* 接收缓冲区大小 */
    socklen_t optlen; /* 选项值长度 */
                      //可优化为HAProxy

    snd_size = SEND_BUFFER_SIZE; /* 发送缓冲区大小为8K */
    optlen = sizeof(snd_size);
    err = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &snd_size, optlen);
    if (err < 0)
    {
        printf("设置发送缓冲区大小错误\n");
    }

    /*
     * 设置接收缓冲区大小
     */
    rcv_size = RECV_BUFFER_SIZE; /* 接收缓冲区大小为8K */
    optlen = sizeof(rcv_size);
    err = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (char *)&rcv_size, optlen);
    if (err < 0)
    {
        printf("设置接收缓冲区大小错误\n");
    }

    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
    {

        perror("fcntl");
        printf("非阻塞模式设置异常\n");
    }

    return fd;
}

int back_tcp_init(const char *ip_addr)
{
    int confd = socket(AF_INET, SOCK_STREAM, 0);
    int err = -1;     /* 返回值 */
    int snd_size = 0; /* 发送缓冲区大小 */
    int rcv_size = 0; /* 接收缓冲区大小 */
    socklen_t optlen; /* 选项值长度 */
    int reuse = 1;

    setsockopt(confd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    /*
     * 设置发送缓冲区大小
     */
    snd_size = SEND_BUFFER_SIZE; /* 发送缓冲区大小为8K */
    optlen = sizeof(snd_size);
    err = setsockopt(confd, SOL_SOCKET, SO_SNDBUF, &snd_size, optlen);
    if (err < 0)
    {
        printf("设置发送缓冲区大小错误\n");
    }

    /*
     * 设置接收缓冲区大小
     */
    rcv_size = RECV_BUFFER_SIZE; /* 接收缓冲区大小为8K */
    optlen = sizeof(rcv_size);
    err = setsockopt(confd, SOL_SOCKET, SO_RCVBUF, (char *)&rcv_size, optlen);
    if (err < 0){
        printf("设置接收缓冲区大小错误\n");
    }

    struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip_addr, &serveraddr.sin_addr.s_addr);
    serveraddr.sin_port = htons(BACNEND_PORT);

    if (connect(confd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        printf("连接server异常\n");
        //  perror("Connect error!");
        close(confd);
        printf("关闭confd为 %d \n", confd);
        return -1;
    }
    //else
    // printf("连接server\n");
    //printf("confd为 %d \n", confd);

    if (fcntl(confd, F_SETFL, O_NONBLOCK) == -1)
    {

        perror("fcntl");
        printf("connect函数非阻塞模式设置异常\n");
    }

    return confd;
}

int tcp_accept(int epoll_fd, int fd)
{
    struct sockaddr_in client_addr
    {
    };
    socklen_t client_length;
    int connect_fd;
    client_length = sizeof(client_addr);
    bzero(&client_addr, client_length);
    connect_fd = accept(fd, (struct sockaddr *)&client_addr, &client_length);
    if (connect_fd < 0)
    {
        perror("BAD ACCEPT\n");
        close(fd);
        return -1;
    }
    // printf("执行accept函数");
    int err = -1;     /* 返回值 */
    int snd_size = 0; /* 发送缓冲区大小 */
    int rcv_size = 0; /* 接收缓冲区大小 */
    socklen_t optlen; /* 选项值长度 */
                      //可优化为HAProxy
    int reuse = 1;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    snd_size = SEND_BUFFER_SIZE; /* 发送缓冲区大小为4M */
    optlen = sizeof(snd_size);
    err = setsockopt(connect_fd, SOL_SOCKET, SO_SNDBUF, &snd_size, optlen);
    if (err < 0)
    {
        printf("设置发送缓冲区大小错误\n");
    }

    /*
     * 设置接收缓冲区大小
     */
    rcv_size = RECV_BUFFER_SIZE; /* 接收缓冲区大小为4M */
    optlen = sizeof(rcv_size);
    err = setsockopt(connect_fd, SOL_SOCKET, SO_RCVBUF, (char *)&rcv_size, optlen);
    if (err < 0)
    {
        printf("设置接收缓冲区大小错误\n");
    }
    if (fcntl(connect_fd, F_SETFL, O_NONBLOCK) == -1)
    {

        perror("fcntl");
        printf("connect函数非阻塞模式设置异常\n");
    }

    epoll_register(EPOLLIN | EPOLLET | EPOLLONESHOT, epoll_fd, connect_fd); // connection_fd 绑定到 epoll| EPOLLET| EPOLLONESHOT

    return connect_fd;
}

// 用这个函数的好处就是自动帮你关闭连接,暂时不关闭连接
int tcp_receive(int client_fd, void *buf, int64_t n)
{

    auto ret = recv(client_fd, buf, n, 0); // 取一个 header 这么大的数据
    //printf("接收函数接收完成fd为%d数据量%d% tid ld\n", client_fd, ret, (long int)syscall(__NR_gettid));
    if (ret < 0)
    {
        // 连接被重置
        if (errno == ECONNRESET)
        {
            perror("ECONNRESET");
        }
        else if (errno == EAGAIN)
        {

            //perror("EAGAIN");
            return -2;
        }
        else
        {
            perror("tcp_receive error");
        }
        //printf("接收函数异常关闭fd为%d\n", client_fd);
        close(client_fd);
        /*分前后端fd
        前端fd执行前后全部断开连接
        后端fd执行负载均衡*/
        if (fdtab.count(client_fd)) //当前存在此fd键值对
        {
            close(fdtab[client_fd]);
            //printf("断开sessionfd%d和fd%d\n", client_fd, fdtab[client_fd]);
            fdtab.erase(fdtab[client_fd]);
            fdtab.erase(client_fd);
        }

        return -1;
    }

    else if (ret == 0)
    {
        // 客户端关闭连接
        close(client_fd);
        if (fdtab.count(client_fd)) //当前存在此fd键值对
        {
            close(fdtab[client_fd]);
            //printf("断开sessionfd%d和fd%d\n", client_fd, fdtab[client_fd]);
            fdtab.erase(fdtab[client_fd]);
            fdtab.erase(client_fd);
        }
        // printf("fd %d connection closed\n", client_fd);
        return -1;
    }

    return ret;
}

int tcp_send(int sock_fd, char *buffer, int64_t length)
{

    while (length > 0)
    {
        //  auto num = send(sock_fd, buffer, length, 0);
        auto num = send(sock_fd, buffer, length, MSG_NOSIGNAL | MSG_DONTWAIT);
        if (num < 0)
        {
            if (errno == EAGAIN)
            {
                //printf("send EAGAIN\n");
                continue;
            }
            printf("senderror  sessionfd%d和fd%d   %ld\n", sock_fd, fdtab[sock_fd], (long int)syscall(__NR_gettid));
            perror("send error:");

            close(sock_fd);
            if (fdtab.count(sock_fd)) //当前存在此fd键值对
            {
                close(fdtab[sock_fd]);
                // printf("断开sessionfd%d和fd%d\n", sock_fd, fdtab[sock_fd]);
                if (fdtab.count(fdtab[sock_fd]))
                    fdtab.erase(fdtab[sock_fd]);
                fdtab.erase(sock_fd);
            }

            return false;
        }
        else
        {
            length -= num;
            buffer += num;
        }
    }

    return 0;
}
