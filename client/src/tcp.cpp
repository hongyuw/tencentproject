//
// Created by Jiashuai on 2020/8/17.
//

#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/mman.h>
#include "../include/tcp.h"
#include <sys/epoll.h>
#include <iostream>
#include "../../etc/config.h"
extern int time_out;

int tcp_init() {

    int server_fd;
    
    // 创建 socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error\n");
        return -1;
    }

    int err = -1;        /* 返回值 */
    int snd_size = 0;   /* 发送缓冲区大小 */
    int rcv_size = 0;    /* 接收缓冲区大小 */
    socklen_t optlen;    /* 选项值长度 */

    /*
     * 先读取缓冲区设置的情况
     * 获得原始发送缓冲区大小
     */
    optlen = sizeof(snd_size);
    err = getsockopt(server_fd, SOL_SOCKET, SO_SNDBUF,&snd_size, &optlen);
    if(err<0){
        perror("error");
        printf("获取发送缓冲区大小错误\n");
    }
    /*
     * 打印原始缓冲区设置情况
     */

    /*
     * 获得原始接收缓冲区大小
     */
    optlen = sizeof(rcv_size);
    err = getsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, &rcv_size, &optlen);
    if(err<0){
        perror("error");
        printf("获取接收缓冲区大小错误\n");
    }

    //printf(" 发送缓冲区原始大小为: %d 字节\n",snd_size);
    //printf(" 接收缓冲区原始大小为: %d 字节\n",rcv_size);

    /*
     * 设置发送缓冲区大小
     */
    snd_size = SEND_BUFF_SIZE;    /* 发送缓冲区大小为8K */
    optlen = sizeof(snd_size);
    err = setsockopt(server_fd, SOL_SOCKET, SO_SNDBUF, &snd_size, optlen);
    if(err<0){
        printf("设置发送缓冲区大小错误\n");
    }

    /*
     * 设置接收缓冲区大小
     */
    rcv_size = RECV_BUFF_SIZE;    /* 接收缓冲区大小为8K */
    optlen = sizeof(rcv_size);
    err = setsockopt(server_fd,SOL_SOCKET,SO_RCVBUF, (char *)&rcv_size, optlen);
    if(err<0){
        printf("设置接收缓冲区大小错误\n");
    }

    /*
     * 检查上述缓冲区设置的情况
     * 获得修改后发送缓冲区大小
     */
    optlen = sizeof(snd_size);
    err = getsockopt(server_fd, SOL_SOCKET, SO_SNDBUF,&snd_size, &optlen);
    if(err<0){
        printf("获取发送缓冲区大小错误\n");
    }

    /*
     * 获得修改后接收缓冲区大小
     */
    optlen = sizeof(rcv_size);
    err = getsockopt(server_fd, SOL_SOCKET, SO_RCVBUF,(char *)&rcv_size, &optlen);
    if(err<0){
        printf("获取接收缓冲区大小错误\n");
    }

    /*
     * 打印结果
     */
    //printf(" 发送缓冲区大小为: %d 字节\n",snd_size);
    //printf(" 接收缓冲区大小为: %d 字节\n",rcv_size);

    struct timeval timeout={time_out,0};
    int re_t=setsockopt(server_fd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
    if (re_t <0 ) {
	printf("failed: time out of send\n");
    }
    re_t = 0;
    re_t=setsockopt(server_fd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
    if (re_t < 0) {
   	printf("failed: time out of recv\n");
    }
    //set_unblock(server_fd);

    return server_fd;
}
int tcp_init_md5() {

    int server_fd;
    
    // 创建 socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error\n");
        return -1;
    }

    int err = -1;        /* 返回值 */
    int snd_size = 0;   /* 发送缓冲区大小 */
    int rcv_size = 0;    /* 接收缓冲区大小 */
    socklen_t optlen;    /* 选项值长度 */

    /*
     * 先读取缓冲区设置的情况
     * 获得原始发送缓冲区大小
     */
    optlen = sizeof(snd_size);
    err = getsockopt(server_fd, SOL_SOCKET, SO_SNDBUF,&snd_size, &optlen);
    if(err<0){
        perror("error");
        printf("获取发送缓冲区大小错误\n");
    }
    /*
     * 打印原始缓冲区设置情况
     */

    /*
     * 获得原始接收缓冲区大小
     */
    optlen = sizeof(rcv_size);
    err = getsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, &rcv_size, &optlen);
    if(err<0){
        perror("error");
        printf("获取接收缓冲区大小错误\n");
    }

    //printf(" 发送缓冲区原始大小为: %d 字节\n",snd_size);
    //printf(" 接收缓冲区原始大小为: %d 字节\n",rcv_size);

    /*
     * 设置发送缓冲区大小
     */
    snd_size = SEND_BUFF_SIZE;    /* 发送缓冲区大小为8K */
    optlen = sizeof(snd_size);
    err = setsockopt(server_fd, SOL_SOCKET, SO_SNDBUF, &snd_size, optlen);
    if(err<0){
        printf("设置发送缓冲区大小错误\n");
    }

    /*
     * 设置接收缓冲区大小
     */
    rcv_size = RECV_BUFF_SIZE;    /* 接收缓冲区大小为8K */
    optlen = sizeof(rcv_size);
    err = setsockopt(server_fd,SOL_SOCKET,SO_RCVBUF, (char *)&rcv_size, optlen);
    if(err<0){
        printf("设置接收缓冲区大小错误\n");
    }

    /*
     * 检查上述缓冲区设置的情况
     * 获得修改后发送缓冲区大小
     */
    optlen = sizeof(snd_size);
    err = getsockopt(server_fd, SOL_SOCKET, SO_SNDBUF,&snd_size, &optlen);
    if(err<0){
        printf("获取发送缓冲区大小错误\n");
    }

    /*
     * 获得修改后接收缓冲区大小
     */
    optlen = sizeof(rcv_size);
    err = getsockopt(server_fd, SOL_SOCKET, SO_RCVBUF,(char *)&rcv_size, &optlen);
    if(err<0){
        printf("获取接收缓冲区大小错误\n");
    }

    /*
     * 打印结果
     */
    //printf(" 发送缓冲区大小为: %d 字节\n",snd_size);
    //printf(" 接收缓冲区大小为: %d 字节\n",rcv_size);

    struct timeval timeout={3000,0};
    int re_t=setsockopt(server_fd,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
    if (re_t <0 ) {
	printf("failed: time out of send\n");
    }
    re_t = 0;
    re_t=setsockopt(server_fd,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));
    if (re_t < 0) {
   	printf("failed: time out of recv\n");
    }
    //set_unblock(server_fd);

    return server_fd;
}

// 用这个函数的好处就是自动帮你关闭连接
int tcp_receive(int client_fd, char *buf, int64_t length) {
    // while(length > 0){
    //     auto num = recv(client_fd, buf, length, 0);
    //     if (num < 0) {
    //         // 连接被重置
    //         if (errno == ECONNRESET) {
    //             perror("ECONNRESET");
    //         } else if (errno == EAGAIN) {
    //             perror("EAGAIN");
    //         } else {
    //             perror("tcp_receive error");
    //         }
    //         return -1;
    //     } else if (num == 0) {
    //         // 客户端关闭连接
    //         //close(client_fd);
    //         printf("client %d connection closed\n", client_fd);
    //         return -1;
    //     }
    //     length -= num;
    //     buf += num;
    // }
    auto num = recv(client_fd, buf, length, 0);
    printf("num : %d , length: %d\n", num, length);
    if (num < 0) {
        // 连接被重置
        if (errno == ECONNRESET) {
            perror("ECONNRESET");
        } else if (errno == EAGAIN) {
            perror("EAGAIN");
        } else {
            perror("tcp_receive error");
        }
        return -1;
    } else if (num == 0) {
        // 客户端关闭连接
        //close(client_fd);
        printf("client %d connection closed\n", client_fd);
        return -1;
    }
    return 0;
}

int tcp_send(int sock_fd, char *buffer, int64_t length) {
    int64_t buf_size = TRANSPORT_BUFF_SIZE_EACH_TIME;
    while (length > 0) {
        int64_t use_size = buf_size <= length ? buf_size : length;
        auto num = send(sock_fd, buffer, use_size, MSG_NOSIGNAL);
        if (num <= 0) {
            perror("send error:");
            return false;
        } else {
            length -= num;
            buffer += num;
        }
    }
    return 0;
}


// success: res = 0
// failed: res = -1
int tcp_connect(int fd, const std::string &ip, const uint16_t port) {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);
    int32_t res = connect(fd, (sockaddr *)&addr, sizeof(addr));
    return res;
  }
