//
// Created by Jiashuai on 2020/8/17.
//


#include <ctime>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/epoll.h>
#include <signal.h>

#include "tcp.h"
#include "worker.h"
#include "../../etc/config.h"

/*
 * 主线程 epoll 处理用户连接
 * accept 之后，开启线程处理文件 上传/下载 请求
 * */
std::map<std::string,char*>* file_name_address_map = nullptr;
std::map<std::string,char*>* file_upload_md5_map = nullptr;

int main(int argc, const char *argv[]) {
    int server_fd;
    if ((server_fd = tcp_init(SERVER_PORT, SERVER_MAX_CONNECTION)) < 0) {
        return 1;
    }

    printf("Server Running on 0.0.0.0:%d\n", SERVER_PORT);

    // 创建 worker 线程
    pthread_t threads[SERVER_MAX_THREADS] = {0};
    for (auto &thread : threads) {
        if (pthread_create(&thread, nullptr, worker_work, nullptr) > 0) {
            perror("pthread_create");
        }
    }

    // ========== 准备好接收客户端连接 ==========

    // 创建一个 epoll
    int epoll_fd;
    if ((epoll_fd = epoll_create(SERVER_MAX_EVENTS)) < 0) {
        perror("epoll create failed\n");
        return 1;
    }

    // 注册 server_fd 到 epoll，这里不能使用 ONESHOT 否则会丢失客户端数据
    epoll_register(EPOLLIN, epoll_fd, server_fd);
    while (true) {
        struct epoll_event events[SERVER_MAX_EVENTS];

        // epoll_wait [错误]返回 -1 ，[超时]返回 0 ，[正常]返回获取到的事件数量
        int ret = epoll_wait(epoll_fd, events, SERVER_MAX_EVENTS, -1);

        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("epoll wait error\n");
                break;
            }
        }

        // 遍历获取到的 epoll 事件
        for (int i = 0; i < ret; i++) {

            auto fd = (int) events[i].data.fd;
            auto ev = (uint32_t) events[i].events;

            if ((ev & EPOLLERR) ||
                (ev & EPOLLHUP) ||
                !(ev & EPOLLIN)) {
                perror("epoll event error\n");
                close(events[i].data.fd);
                continue;
            } else if ((fd == server_fd) && (ev & EPOLLIN)) {
                tcp_accept(epoll_fd, fd);
            } else if (ev & EPOLLIN) {
                // 有 客户端 的 fd 收到 数据
                worker_put(fd);
            } else {

            }
        }
    }

    close(epoll_fd);
    close(server_fd);
    return 0;
}

