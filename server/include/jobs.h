
#ifndef _JOBS_H
#define _JOBS_H


#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <condition_variable>
#include <map>

#include "tcp.h"
#include "../../etc/config.h"
#include "../include/md5.h"

#define ADDJOB(type, handler) {type, handler}
#define MAX_JOBS MSG_TYPE_NUM // 与 MSG_TYPE 有关

typedef int (*HandlerFunc)(int socket_fd, Package *p);

struct job {
    MSG_TYPE type;
    HandlerFunc handler;
};
extern std::map<std::string,char*>* file_name_address_map;
extern struct job jobs[MSG_TYPE_SIZE - MSG_TYPE_INIT_STATUS - 1];


// Handler Function Definitions ↓
// 不用管 socket_fd 和 Package *p 的释放，外面会帮你释放的
// 别自己手动释放

int job_download_send_file_info(int socket_fd, Package *p);

int job_download_send_block_content(int socket_fd, Package *p);

int job_download_unmap_file(int socket_fd, Package *p);

int job_download_breakpoint_recv(int socket_fd, Package *p);

int job_upload_recv_file_info(int socket_fd, Package *p);

int job_upload_recv_block_content(int socket_fd, Package *p);

int job_upload_breakpoint_send(int socket_fd, Package *p);

int job_upload_unmap_file(int socket_fd, Package *p);

int job_cal_md5(int socket_fd, Package *p);

#endif
