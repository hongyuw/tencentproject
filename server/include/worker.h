//
// Created by Jiashuai on 2020/8/17.
//

#ifndef SERVER_WORKER_H
#define SERVER_WORKER_H


#include <mutex>
#include <queue>
#include <thread>
#include <cassert>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <condition_variable>

#include "tcp.h"
#include "jobs.h"
#include "../../etc/config.h"

void worker_put(int fd);

void *worker_work(void *ptr);

extern std::mutex __mutex;
extern std::queue<int> worker_queue;
extern std::condition_variable worker_condition; // 全局 条件变量




#endif //SERVER_WORKER_H
