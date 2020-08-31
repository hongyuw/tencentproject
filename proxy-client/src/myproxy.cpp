#include "../include/ev_epoll.h"
#include "../include/protocol.h"
#include "../include/worker.h"

//static void init(int argc, char **argv)读取命令行

int epfd; //epoll专用文件描述符

int ffd; //用于前端监听
//int back_fd;  //用于后端监听
//struct fdtab *fdtab = nullptr; /* array of all the file descriptors */
extern pthread_mutex_t worker_mutex;

static void init() //初始化连接前端和后端
{
    epfd = epoll_init();
    //printf("epoll init\n");
    ffd = front_tcp_init(FRONTEND_PORT, FRONTEND_MAX_CONNECTION); //做server接收前端连接
                                                                  // printf("前端初始化开始监听\n");

    // back_fd = back_tcp_init(BACKEND_IP_ADDR_1); //做client连接后端
    //printf("后端初始化连接server\n");
    epoll_register(EPOLLIN, epfd, ffd); //   epoll_register(EPOLLIN, epfd, front_fd);
                                        // printf("front_fd加入epoll\n");
    worker_mutex_init(worker_mutex);

    /* do something */
    //两个proxy还是有区别的，server不会挂第二个proxy可以直接连接，而第一个proxy要做健康检查
    // epoll_register(EPOLLIN, epfd, back_fd);
    //printf("back_fd加入epoll\n");
}
//static void *run_thread_poll_loop(void *data)
void run_main_loop()
{
    // printf("开始主循环\n");
    _do_poll(epfd);
}

int main(int argc, const char *argv[])
{

    init();
    pthread_t threads[SERVER_MAX_THREADS] = {0};
    for (auto &thread : threads)
    {
        // printf("创建线程\n");
        if (pthread_create(&thread, nullptr, worker_work, nullptr) > 0)
        {
            perror("pthread_create");
        }
    }
    //printf("struct %d", sizeof(Clientmsg));
    while (1)
    {
        run_main_loop();
    }
    close(epfd);
    close(ffd);
}