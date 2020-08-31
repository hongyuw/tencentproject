#ifndef WORKER_H
#define WORKER_H
#include <pthread.h>
#include <queue>
#include <cassert>

#include "../include/ev_epoll.h"

void worker_mutex_init(pthread_mutex_t mutex);
void worker_put(int fd);
void *worker_work(void *ptr);

#endif
