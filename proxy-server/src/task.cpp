#include "../include/task.h"

TaskQueue::TaskQueue()
{
    pthread_mutex_init(&taskqueue_mutex, NULL);
}

TaskQueue::~TaskQueue()
{
    pthread_mutex_destroy(&taskqueue_mutex);
}

void TaskQueue::addTask(Task &task)
{
    pthread_mutex_lock(&taskqueue_mutex);
    taskqueue_queue.push(task);
    pthread_mutex_unlock(&taskqueue_mutex);
}

Task TaskQueue::takeTask()
{

    pthread_mutex_lock(&taskqueue_mutex);
    if (taskqueue_queue.size() > 0)
    {
        Task t = taskqueue_queue.front();
        taskqueue_queue.pop();
        pthread_mutex_unlock(&taskqueue_mutex);
        return t;
    }
    else
        pthread_mutex_unlock(&taskqueue_mutex);
    return Task();
}