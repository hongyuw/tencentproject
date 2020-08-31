#include "../../etc/config.h"
#include "../include/tcp.h"
#include "../include/small_file.h"

#include <assert.h>
#include <sys/mman.h>

int do_small_file_upload_new(char *file_name, size_t file_size) {
    printf("enter small file upload\n");
    Package* head_package = new Package(UPLOAD_SEND_FILE_INFO, SMALL_FILE, file_name, file_size, 0);
    
    std::string file_path = std::string(file_name);
    int filefd = open(file_path.c_str(), O_RDONLY, 0777);
    char *mmap_ptr = (char *)mmap(nullptr, file_size, PROT_READ , MAP_SHARED, filefd, 0);
    std::cout << "do_small_file_upload_new:: open file success" << std::endl;

    int connect = 1;
    
    // 上传:阶段I -> 向服务器发送文件信息， 客户端发送完毕后需等待服务端的回包          
    // 发文件信息包
    while (true){
        int ret;
        int socket_fd = tcp_init();

        //初始化服务端
        if (connect == 0){
            ret = tcp_connect(socket_fd, SERVER_IP_ADDR_1, SERVER_PORT);
            if (-1 == ret)
            {
                std::cout << "do_small_file_upload_new:: tcp connect server 1 failed, try server 2" << std::endl;
                close(socket_fd);
                connect = abs(connect - 1);
                continue;
            }
        }
        else{
            ret = tcp_connect(socket_fd, SERVER_IP_ADDR_2, SERVER_PORT);
            if (-1 == ret)
            {
                std::cout << "do_small_file_upload_new:: tcp connect server 2 failed, try server 1" << std::endl;
                close(socket_fd);
                connect = abs(connect - 1);
                continue;
            }
        }
        printf("连接服务端成功....\n");

        // 发头包
        Package* head_package = new Package(UPLOAD_SEND_FILE_INFO, SMALL_FILE,(char*)file_name,file_size, 0, 0);
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

        auto *p = (Package*) buf;
        if(p->msg_type == OK){
            printf("server upload file info OK\n");
        }
        else
        {
            printf("server upload file info not OK\n");
            continue;
        }
        printf("收包头的回复（OK）成功....\n");
        
        // 发送包体
        ret = tcp_send(socket_fd, mmap_ptr, file_size);
        if (ret == -1){
            printf("do_small_file_upload_new:: baoti send error\n");
            close(socket_fd);
            continue;
        }
        printf("do_small_file_upload_new:: baoti send success\n");

        // 收包体的回复
        char new_buf[sizeof(Package)] = {0};
        ret = tcp_receive(socket_fd, new_buf, Package_len);
        // ret = recv(socket_fd, buf, Package_len, 0);
        printf("package len: %d\n", Package_len);
        if (ret == -1){
            close(socket_fd);
            continue;
        }
        printf("收包体的回复成功....\n");
        close(socket_fd);

        p = (Package*) new_buf;
        if(p->msg_type == OK){
            printf("server upload file body OK\n");
        }
        else
        {
            printf("server upload file body not OK\n");
            continue;
        }
        printf("收包体的回复（OK）成功....\n");
        break; 
    }

    printf("sending file_upload_over\n");
    while (true){
      int socket_fd = tcp_init();
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
      printf("sending file_upload_ove::连接服务端成功....\n");

      // 发头包
      Package* head_package = new Package(MD5_CAL, SMALL_FILE,(char*)file_name,file_size, 0, 0);
      ret = tcp_send(socket_fd, (char*)head_package, Package_len);
      if (ret == -1){
        close(socket_fd);
        continue;
      }
      printf("send file_upload_over success\n");
      break;
    }
    return 0;
}