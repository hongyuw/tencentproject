#pragma once

#define MAX_EVENTS 500              // epoll 设置
#define FRONTEND_MAX_CONNECTION 500 // 前端 listen 设置
#define BACNEND_MAX_CONNECTION 500  // 后端 listen 设置

#define SEND_BUFFER_SIZE 16777216
#define RECV_BUFFER_SIZE 16777216

#define FUNC_BUFFER 10240

#define FRONTEND_PORT 6667 // 监听端口
#define BACNEND_PORT 6667  // 后端服务器端口

#define BACKEND_IP_ADDR_1 "100.116.210.27" // 后端IP//9.134.235.227  1234
#define BACKEND_IP_ADDR_2 "100.116.210.31"
//9.134.234.58 6667 本机server
//server  9.56.16.44

#define SERVER_MAX_THREADS 24
