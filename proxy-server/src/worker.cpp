/*解析包头并确定接收和发送的大小，然后执行转发功能*/

#include "../include/worker.h"

std::queue<int> worker_queue;
pthread_mutex_t worker_mutex;
pthread_cond_t worker_cond = PTHREAD_COND_INITIALIZER;
extern std::unordered_map<int, int> fdtab;
#define __NR_gettid 186
extern int epfd;

void worker_mutex_init(pthread_mutex_t mutex)
{
    pthread_mutex_init(&worker_mutex, NULL);
}

void worker_put(int fd)
{
    pthread_mutex_lock(&worker_mutex);
    //printf("fd进入队列%d\n", fd);
    worker_queue.push(fd);
    pthread_cond_broadcast(&worker_cond);
    pthread_mutex_unlock(&worker_mutex);
}

void *worker_work(void *ptr)
{
    char buffer[FUNC_BUFFER];
    while (true)
    {

        pthread_mutex_lock(&worker_mutex);
        // printf("工作加锁  pid %ld\n", (long int)syscall(__NR_gettid));
        while (worker_queue.empty())
        {
            // printf("等待队列不为空或信号\n");
            pthread_cond_wait(&worker_cond, &worker_mutex);
        }
        assert(!worker_queue.empty());

        int work_fd = worker_queue.front();
        worker_queue.pop();
        pthread_mutex_unlock(&worker_mutex);
        // printf("工作解锁 pid %ld\n", (long int)syscall(__NR_gettid));

        while (1)
        {
            // printf("包体接收pid %ld\n", (long int)syscall(__NR_gettid));
            auto ret = tcp_receive(work_fd, buffer, sizeof(buffer));
            if (ret == 0)
            {
                // printf("数据接收完成pid %ld\n", (long int)syscall(__NR_gettid));
                break;
            }
            // printf("数据内容为%s\n", buffer);
            //for (int index = 0; index < sizeof(Clientmsg); index++)
            // std::cout << buffer[index];
            //如果不断开连接，不能使用此封装的函数
            if (ret == -1)
            {
                // printf("Error occurred!pid %ld\n", (long int)syscall(__NR_gettid));
                break;
            }

            if (ret == -2)
            {
                // printf("EAGAIN\n");
                reset_oneshot(epfd, work_fd);
                //continue;
                break;
            }

            //void* largebuffer = (char*)malloc(81920*sizeof(char));
            // printf("包体发送pid %ld\n", (long int)syscall(__NR_gettid));
            auto sendret = tcp_send(fdtab[work_fd], buffer, ret);
            /*if(false)
                
                    1.choose new server(不知道第二个proxy发了多少缓冲区)
                    so
                    tell client 重新发fd对应的block
                    
                          
                */
        }
    }
}