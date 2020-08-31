#include "../include/big_file.h"
#include <cstring>
#include <pthread.h>
#include <mutex>
#include "../include/tcp.h"


std::mutex m;
int upload_okblock = 0;
/*
 * thread func
 */

void *thr_upload_start_new(void *arg) {
  ThreadArgPtr tupPtr = *((ThreadArgPtr *)arg);
  off_t offset;
  char *file_name;
  int32_t fd, block_no;
  int64_t real_block_size;
  int64_t file_size;
  char *mmap_ptr;
  std::tie(file_name, fd, offset, real_block_size, block_no,file_size, mmap_ptr) = *tupPtr;
  int connect = block_no % 2;
  
  while(true){
    int socket_fd = tcp_init();
    connect = abs(connect -1);

    //初始化服务端
    int ret;
    if (connect == 0)
    {
      ret = tcp_connect(socket_fd, SERVER_IP_ADDR_1, SERVER_PORT);
      if (ret == -1){
        printf("thr_upload_start_new:: thread %d tcp connect server 1 error\n", block_no);
        close(socket_fd);
        continue;
      }
    }
    else
    {
      ret = tcp_connect(socket_fd, SERVER_IP_ADDR_2, SERVER_PORT);
      if (ret == -1){
        printf("thr_upload_start_new:: thread %d tcp connect server 2 error\n", block_no);
        close(socket_fd);
        continue;
      }
    }
        
    // 上传阶段II 发包头
    Package* package = new Package(UPLOAD_SEND_BLOCK_INFO, BIG_FILE, file_name, file_size, block_no, 0);
    ret = tcp_send(socket_fd, (char*)package, Package_len);
    if (ret == -1){
      printf("thr_upload_start_new:: thread %d baotou send error\n", block_no);
      close(socket_fd);
      continue;
    }
    printf("thr_upload_start_new:: thread %d baotou send success\n", block_no);

    // 上传阶段II 发包头信息收回包
    char buf_upload_big[sizeof(Package)] = {0};
    ret = tcp_receive(socket_fd, buf_upload_big, Package_len);
    if (ret == -1){
      printf("thr_upload_start_new:: thread %d yigebao recv error\n", block_no);
      close(socket_fd);
      continue;
    }
    printf("thr_upload_start_new:: thread %d yigebao recv success\n", block_no);

    auto *p_upload_big = (Package*) buf_upload_big;
    if(p_upload_big->msg_type == OK){
      printf("thr_upload_start_new:: thread %d baotou(OK) recv success\n", block_no);
    }
    else{
      printf("thr_upload_start_new:: thread %d baotou(OK) recv error\n", block_no);
      close(socket_fd);
      continue;
    }

    // 上传阶段II 发包体
    ret = tcp_send(socket_fd, mmap_ptr, real_block_size);
    if (ret == -1){
      printf("thr_upload_start_new:: thread %d baoti send error\n", block_no);
      close(socket_fd);
      continue;
    }
    printf("thr_upload_start_new:: thread %d baoti send success\n", block_no);

    // 上传阶段II 等确认
    char buf[sizeof(Package)] = {0};
    ret = tcp_receive(socket_fd, buf, Package_len);
    if (ret == -1){
      printf("thr_upload_start_new:: thread %d baoti recv error\n", block_no);
      close(socket_fd);
      continue;
    }
    printf("thr_upload_start_new:: thread %d baoti recv success\n", block_no);

    auto *p = (Package*) buf;
    if(p->msg_type == OK){
      printf("thr_upload_start_new:: thread %d baoti(OK) recv success\n", block_no);
      ++ upload_okblock;
      break;
    }
    else{
      printf("thr_upload_start_new:: thread %d baoti(OK) recv error\n", block_no);
      close(socket_fd);
      continue;
    }
  }
  return nullptr;
}

void do_big_file_upload_new(const char *file_name, const uint64_t file_size){

  // todo: file fd当成参数传进线程
  const uint64_t thr_num = SERVER_MAX_THREADS;
  const uint64_t block_size = file_size / thr_num;
  const uint64_t last_block = file_size % thr_num;

  std::string file_path = std::string(file_name);
  int filefd = open(file_path.c_str(), O_RDONLY, 0777);
  char *mmap_ptr = (char *)mmap(nullptr, file_size, PROT_READ , MAP_SHARED, filefd, 0);
  std::cout << "do_big_file_upload_new:: open file and mmap success" << std::endl;

  auto *tid = new pthread_t[thr_num];
  std::vector<ThreadArgPtr> vec(thr_num);

  // 上传:阶段I -> 向服务器发送文件信息， 客户端发送完毕后需等待服务端的回包
  while(true)
  {
    int socket_fd = tcp_init();

    //初始化服务端
    int ret = tcp_connect(socket_fd, SERVER_IP_ADDR_1, SERVER_PORT);
    if (-1 == ret)
    {
      std::cout << "do_big_file_upload_new:: tcp connect server 1 failed, try server 2" << std::endl;
      close(socket_fd);
      socket_fd = tcp_init();
      ret = tcp_connect(socket_fd, SERVER_IP_ADDR_2, SERVER_PORT);
      if (ret == -1){
        std::cout << "do_big_file_upload_new:: tcp connect server 2 failed" << std::endl;
        close(socket_fd);
        continue;
      }
    }
    printf("连接服务端成功....\n");

    // 发头包
    Package* head_package = new Package(UPLOAD_SEND_FILE_INFO, BIG_FILE,(char*)file_name,file_size, 0, 0);
    ret = tcp_send(socket_fd, (char*)head_package, Package_len);
    if (ret == -1){
      close(socket_fd);
      continue;
    }
    printf("发送包头成功....\n");   
    
    // 收包头的回复
    char buf[sizeof(Package)] = {0};
    ret = tcp_receive(socket_fd, buf, Package_len);
    // ret = recv(socket_fd, buf, Package_len, 0);
    printf("package len: %d\n", Package_len);
    if (ret == -1){
      close(socket_fd);
      continue;
    }
    printf("收包头的回复成功....\n");
    close(socket_fd);

    auto *p = (Package*) buf;
    if(p->msg_type == OK){
      printf("server upload file info OK\n");
      break;
    }
    else
    {
      printf("server upload file info not OK\n");
      continue;
    }
  }

  // 创建线程，开始发送block
  for (uint32_t i = 0; i < thr_num; ++i) {
    off_t offset = i * block_size;
    uint64_t real_block_size =
        (i == thr_num - 1) ? (block_size + last_block) : block_size;

    ThreadArgPtr arg =
        ThreadArgPtr(new ThreadArg((char*)file_name, 0, offset, real_block_size, i, file_size, mmap_ptr));
    vec[i] = arg;

    int32_t res = pthread_create(&tid[i], nullptr, thr_upload_start_new, (void *)&(vec[i]));
    mmap_ptr += real_block_size;
    if (res != 0) {
      std::cerr << "(big_file_upload)创建第i=" << i--
                << "个线程时失败,pthread_id=" << pthread_self() << std::endl;
      mmap_ptr -= real_block_size;
    }
    printf("all threads create success\n");
  }
  
  printf("before join\n");
  for (uint64_t i = 0; i < thr_num; ++i) {
    pthread_join(tid[i], nullptr);
  }

  printf("after join\n");
  if(upload_okblock == SERVER_MAX_THREADS){
    printf("upload success, package send %d/%d\n",upload_okblock,SERVER_MAX_THREADS);
    printf("sending file_upload_over\n");
    while (true){
      int socket_fd = tcp_init_md5();
      int ret = tcp_connect(socket_fd, SERVER_IP_ADDR_1, SERVER_PORT);
      if (-1 == ret)
      {
        std::cout << "do_big_file_upload_new:: tcp connect server 1 failed, try server 2" << std::endl;
        close(socket_fd);
        socket_fd = tcp_init_md5();
        ret = tcp_connect(socket_fd, SERVER_IP_ADDR_2, SERVER_PORT);
        if (ret == -1){
          std::cout << "do_big_file_upload_new:: tcp connect server 2 failed" << std::endl;
          close(socket_fd);
          continue;
        }
      }
      printf("sending file_upload_over::连接服务端成功....\n");

      // 发头包
      Package* head_package = new Package(MD5_CAL, BIG_FILE,(char*)file_name,file_size, 0, 0);
      ret = tcp_send(socket_fd, (char*)head_package, Package_len);
      if (ret == -1){
        close(socket_fd);
        continue;
      }

      // OK
      char new_buf[sizeof(Package)] = {0};
      ret = tcp_receive(socket_fd, new_buf, Package_len);
      printf("package len: %d\n", Package_len);
      if (ret == -1){
          close(socket_fd);
          continue;
      }
      printf("sending file_upload_over::收包体的回复成功....\n");
      close(socket_fd);

      auto p = (Package*) new_buf;
      if(p->msg_type == OK){
          printf("sending file_upload_over::server upload file body OK\n");
      }
      else
      {
          printf("sending file_upload_over::server upload file body not OK\n");
          continue;
      }
      printf("sending file_upload_over::收包体的回复（OK）成功....\n");
      printf("send file_upload_over success\n");
      break;
    }
  }
  else
  {
    printf("upload not success, package send %d/%d\n",upload_okblock,SERVER_MAX_THREADS);
  }

}
