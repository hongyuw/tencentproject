#pragma once
#include <queue>
#include <pthread.h>

struct Task
{
    Task()
    {
        function = nullptr;
        arg = nullptr;
    }
    void *(*function)(void *); /* 函数指针，回调函数 */
    void *arg;                 /* 上面函数的参数 */
};

class TaskQueue
{
public:
    TaskQueue();
    ~TaskQueue();
    void addTask(Task &task);
    Task takeTask();

private:
    pthread_mutex_t taskqueue_mutex;
    std::queue<Task> taskqueue_queue;
};