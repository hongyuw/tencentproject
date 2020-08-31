//
// Created by Jiashuai on 2020/8/17.
//
#ifndef SERVER_CONFIG_H
#define SERVER_CONFIG_H
#pragma once
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define SMALL_FILE_SIZE_MAX 10 * 1024 * 1024 

#define SERVER_IP_ADDR_1  "100.65.100.99" // 客户端要连接的第一个ip 9.134.234.148 9.134.233.66 9.134.237.47
#define SERVER_IP_ADDR_2  "100.65.100.50" // 客户端要连接的第二个ip  proxy:9.134.234.58 9.134.234.46

#define SEND_BUFF_SIZE 4096 * 1024
#define RECV_BUFF_SIZE 4096 * 1024
#define TRANSPORT_BUFF_SIZE_EACH_TIME 10240//16384
#define TIME_OUT_VALUE 5
// 服务端相关配置
#define SERVER_PORT 6667           // 监听端口
#define SERVER_MAX_CONNECTION 500  // epoll 设置
#define SERVER_MAX_EVENTS     500  // epoll 设置
#define SERVER_MAX_THREADS    12   // 服务端有多少线程处理文件分块（一个分块一个线程）
#define BLOCK_TOTAL_NUM   12       // 文件分成多少块
#define SERVER_FILE_SAVE_PATH    "/data/group1/file/"   //"/root/project/"

// 下方为公共部分
enum MSG_TYPE : int8_t {
    MSG_TYPE_INIT_STATUS = 0,
    DOWNLOAD_SEND_FILE_NAME,     //下载:阶段I -> 客户端向服务器发送 文件信息请求，         发送完毕后需等待 服务器返回文件信息
    DOWNLOAD_SEND_BLOCK_NUM,     //下载:阶段II -> 向服务器发送 要下载的大文件的BLOCK_NUM, 发送完毕后需要等待 服务器返回对应的文件内容, 如果需要断点续传，只需要客户端在包头中，设置好off_set即可
    DOWNLOAD_FILE_OVER,          //下载:阶段III -> 全部文件接收完毕，客户端需要发送给服务端一个信息
    UPLOAD_SEND_FILE_INFO,       //上传:阶段I -> 向服务器发送 文件信息， 客户端发送完毕后需等待服务端的回包
    UPLOAD_SEND_BLOCK_INFO,      //上传:阶段II -> 向服务器发送 块信息， 等回包后 再发主体， 再等回包确认接收完毕， 如果没有收到回包，或者中途断开，则发送断点续传请求
    UPLOAD_BREAKPOINT_QUERY,     //上传:断点查询：意外断开连接后，客户端询问服务端，某blockNo的偏移量，服务端会回复off_set(断点续传)
    UPLOAD_FILE_OVER,           //上传完毕，客户端发包给服务端，告知上传完毕

    MD5_CAL,                    //计算文件md5
    MSG_TYPE_SIZE,               //用于记录一共有多少中msg_type
    OK,                  //回复:成功
    FAILED,              //回复:失败
};


enum FILE_TYPE : int8_t {
    FILE_TYPE_INIT_STATUS = 0,
    BIG_FILE,   //大文件
    SMALL_FILE,     //小文件
};


struct Package {
    MSG_TYPE msg_type;
    FILE_TYPE file_type;
    char file_name[256];
    char md5[50];
    uint64_t file_size;
    uint32_t block_now_num;
    uint64_t off_set;
    Package(MSG_TYPE tp = MSG_TYPE_INIT_STATUS, FILE_TYPE ftp = FILE_TYPE_INIT_STATUS,
            char *fname = nullptr, uint64_t fsize = 0, uint32_t block = 0, uint64_t ofst = 0, char* md5_ = nullptr)
            : msg_type(tp), file_type(ftp), file_size(fsize), block_now_num(block),off_set(ofst) {
        // bzero((void *)file_name, sizeof(file_name));
        memset(file_name, 0, sizeof(file_name));
        memset(md5,0,sizeof(md5));
        if (fname) strcpy(file_name, fname);
        if (md5_) strcpy(md5, md5_);
    }
};


#define MSG_TYPE_NUM  MSG_TYPE_SIZE - MSG_TYPE_INIT_STATUS + 1
const u_int32_t Package_len = sizeof(Package);
#endif //SERVER_CONFIG_H
