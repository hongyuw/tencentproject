//
// Created by Jiashuai on 2020/8/17.
//

#include "../include/worker.h"


std::mutex __mutex;
std::queue<int> worker_queue;
std::condition_variable worker_condition; // 全局 条件变量
// 新的处理任务
void worker_put(int fd) {
    {
        __mutex.lock();
        worker_queue.push(fd);
        worker_condition.notify_one();
        __mutex.unlock();
    }
}


// 循环从队列里取出任务，解析包头，判断类型，然后交给 job.handler 处理
void *worker_work(void *ptr) {
    while (true) {
        std::unique_lock<std::mutex> lock(__mutex);
        while (worker_queue.empty()) {
            worker_condition.wait(lock);
        }
        assert(!worker_queue.empty());

        int job_fd = worker_queue.front();
        worker_queue.pop();
        lock.unlock();
        // ================= ↑ 取出一个任务 ↑ =================

        // 接收包头
        unsigned char buffer[sizeof(Package)] = {0};
        if (tcp_receive(job_fd, buffer, sizeof(Package)) < 0) {
            continue;
        }

        // 解析包头
        auto *p = (Package *) buffer;

        //printf("==============================================\n");
        //printf("          PACKAGE HEADER RECEIVED             \n");
        printf("Message Type:\t\t");
        switch ((MSG_TYPE) p->msg_type) {
            case MSG_TYPE_INIT_STATUS:
                printf("INIT_STATUS\n");
                break;
            case DOWNLOAD_SEND_FILE_NAME:
                printf("DOWNLOAD: SEND_FILE_NAME\n");
                break;
            case DOWNLOAD_SEND_BLOCK_NUM:
                printf("DOWNLOAD: SEND_BLOCK_NUM\n");
                break;
            case DOWNLOAD_FILE_OVER:
                printf("DOWNLOAD: DOWNLOAD_FILE_OVER\n");
                break;
            case UPLOAD_SEND_FILE_INFO:
                printf("UPLOAD: SEND_FILE_INFO\n");
                break;
            case UPLOAD_SEND_BLOCK_INFO:
                printf("UPLOAD: SEND_BLOCK_INFO\n");
                break;
            case UPLOAD_FILE_OVER:
                printf("UPLOAD: FILE_OVER\n");
                break;
            case MD5_CAL:
		printf("MD5 !!!!\n");
		break;
        }
        //printf("File Name:\t\t%s \n", p->file_name);
        //printf("File Size:\t\t%llu\n",p->file_size);
        //printf("Block  No:\t\t%d\n",p->block_now_num);
        //printf("off_set:  \t\t%llu\n",p->off_set);
        //printf("File Type:\t\t");
        switch ((FILE_TYPE) p->file_type) {
            case FILE_TYPE_INIT_STATUS:
                //printf("INIT_STATUS\n");
                break;
            case BIG_FILE:
                //printf("BIG_FILE\n");
                break;
            case SMALL_FILE:
                //printf("SMALL_FILE\n");
                break;
        }

        //printf("==============================================\n");

        // 根据控制码调用相关的处理函数
        // 包括继续接收后续的包，也由 handler 进行处理
        for (auto &job : jobs) {
            if (job.type == (MSG_TYPE) p->msg_type) {
                job.handler(job_fd, p);
            }
        }


        close(job_fd);
        // 任务执行完毕，关闭 job_fd
        // 任务 = 包头 + payload
    }
    return nullptr;
}

