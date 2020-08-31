#include "../../etc/config.h"
#include "../include/tcp.h"
#include "../include/small_file.h"

#include <assert.h>
#include <sys/mman.h>


int do_small_file_upload(TcpSocket &socket_fd, int fd, char *file_name, size_t file_size, int &ret) {
    printf("enter small file upload\n");

    // 上传:阶段I -> 向服务器发送文件信息， 客户端发送完毕后需等待服务端的回包
    while(true){
        try{            
            // 发文件信息包
            std::shared_ptr<Package> head_package(
                new Package(UPLOAD_SEND_FILE_INFO, SMALL_FILE, file_name, file_size, 0));
            if( !socket_fd.Send((void *)&(*head_package), sizeof(Package)) )
            {
                socket_fd.Close();
                socket_fd.Socket();
                if(! (socket_fd.Connect(SERVER_IP_ADDR_1, SERVER_PORT)) )
                {
                    socket_fd.Close();
                    socket_fd.Socket();
                    bool Connected_Change = socket_fd.Connect(SERVER_IP_ADDR_2, SERVER_PORT);
                    std::cout<<"clientInit to SERVER_IP_ADDR_2"<<"\n";
                    std::cout<<"Connected_Change: "<<Connected_Change<<"\n";
                }         
                std::cout<<"Send File Info Faild\n";
            }
            std::cout<<"file_size: "<<file_size<<"\n";

            char buf_small_upload_info[sizeof(Package)] = {0};
            if(socket_fd.Recv(&buf_small_upload_info, sizeof(Package))<=0)
            {
                std::cout<<"Recv File Info OK Faild\n";
                continue;
            }
            
            auto *p_small_upload = (Package*) buf_small_upload_info;

            std::cout<<"send file info return msg_type: "<<p_small_upload->msg_type<<"\n";

            if(p_small_upload->msg_type != OK){
                printf("server info not ok\n");
                continue;
            }
            else{
                printf("server info ok\n");
            }


            std::string filedir = "../upload/";

            std::string file_path = std::string(filedir) + std::string(file_name);
    
            int fd = open(file_path.c_str(), O_RDONLY, 0777);
            printf("file fd: %d\n",fd);
            truncate64(file_name, file_size);
            
            char *mmap_ptr = (char *)mmap(nullptr, file_size, PROT_READ , MAP_SHARED, fd, 0);
            if (mmap_ptr == MAP_FAILED) {
                perror("mmap error!");
                return -1;
            }

            // 上传阶段II 发包体
            int upload_body_try = 0;
            while (upload_body_try<3)
            {
                if(upload_body_try == 3)
                {
                    return -1;
                }
                std::cout << "\nenter upload_body while"<<"\n";
                
                //这里改成什么函数会比较好？
                //tcp_send(socket_fd.GetFd(), mmap_ptr, file_size);
                if( !socket_fd.Send((void *)&(*mmap_ptr), file_size) )
                {
                    socket_fd.Close();
                    socket_fd.Socket();
                    if(! (socket_fd.Connect(SERVER_IP_ADDR_1, SERVER_PORT)) )
                    {
                        socket_fd.Close();
                        socket_fd.Socket();
                        bool Connected_Change = socket_fd.Connect(SERVER_IP_ADDR_2, SERVER_PORT);
                        std::cout<<"clientInit to SERVER_IP_ADDR_2"<<"\n";
                        std::cout<<"Connected_Change: "<<Connected_Change<<"\n";
                    }         
                    std::cout<<"Send File Body Faild\n";
                }
                std::cout << " === SMALL_FILE BODY SENT ===\n";

                // 上传阶段II 等确认
                char buf_small_upload_body[sizeof(Package)] = {0};
                if(socket_fd.Recv(&buf_small_upload_body, sizeof(Package)) <= 0)
                {
                    std::cout << "\nBody Recv Not OK\n";
                    ++ upload_body_try;
                    continue;
                }
                else
                {
                    std::cout << "\nBody Recv OK\n";
                }
                

                auto *p = (Package*) buf_small_upload_body;
                if(p->msg_type == OK)
                {
                    Package *pp(new Package(UPLOAD_FILE_OVER, SMALL_FILE, file_name, file_size, 0));
                    if( !socket_fd.Send((void *)pp, sizeof(Package)) )
                    {
                        std::cout<<"Send File OVER Faild\n";
                        socket_fd.Close();
                        socket_fd.Socket();
                        if(! (socket_fd.Connect(SERVER_IP_ADDR_1, SERVER_PORT)) )
                        {
                            socket_fd.Close();
                            socket_fd.Socket();
                            bool Connected_Change = socket_fd.Connect(SERVER_IP_ADDR_2, SERVER_PORT);
                            std::cout<<"clientInit to SERVER_IP_ADDR_2"<<"\n";
                            std::cout<<"Connected_Change: "<<Connected_Change<<"\n";
                        }
                    }
                    ret = 0;
                    return 0; 
                }    
                //如果传包不ok就continue
                else 
                {
                    std::cout << "SMALL_FILE 上传阶段II 等确认 not ok\n";
                    ++ upload_body_try;
                    continue;
                }
            }
        }
        catch(...){
            std::cout << "SMALL_FILE UPLOAD ERROR\n";
            return -1;
        }
    }
}


int do_small_file_upload_new(TcpSocket &socket_fd, int fd, char *file_name, size_t file_size, int &ret) {
    printf("enter small file upload\n");

    // 上传:阶段I -> 向服务器发送文件信息， 客户端发送完毕后需等待服务端的回包
    while(true){
        try{            
            // 发文件信息包
            std::shared_ptr<Package> head_package(
                new Package(UPLOAD_SEND_FILE_INFO, SMALL_FILE, file_name, file_size, 0));
            if( !socket_fd.Send((void *)&(*head_package), sizeof(Package)) )
            {
                socket_fd.Close();
                socket_fd.Socket();
                if(! (socket_fd.Connect(SERVER_IP_ADDR_1, SERVER_PORT)) )
                {
                    socket_fd.Close();
                    socket_fd.Socket();
                    bool Connected_Change = socket_fd.Connect(SERVER_IP_ADDR_2, SERVER_PORT);
                    std::cout<<"clientInit to SERVER_IP_ADDR_2"<<"\n";
                    std::cout<<"Connected_Change: "<<Connected_Change<<"\n";
                }         
                std::cout<<"Send File Info Faild\n";
            }
            std::cout<<"file_size: "<<file_size<<"\n";

            char buf_small_upload_info[sizeof(Package)] = {0};
            if(socket_fd.Recv(&buf_small_upload_info, sizeof(Package))<=0)
            {
                std::cout<<"Recv File Info OK Faild\n";
                continue;
            }
            
            auto *p_small_upload = (Package*) buf_small_upload_info;

            std::cout<<"send file info return msg_type: "<<p_small_upload->msg_type<<"\n";

            if(p_small_upload->msg_type != OK){
                printf("server info not ok\n");
                continue;
            }
            else{
                printf("server info ok\n");
            }


            std::string filedir = "../upload/";

            std::string file_path = std::string(filedir) + std::string(file_name);
    
            int fd = open(file_path.c_str(), O_RDONLY, 0777);
            printf("file fd: %d\n",fd);
            truncate64(file_name, file_size);
            
            char *mmap_ptr = (char *)mmap(nullptr, file_size, PROT_READ , MAP_SHARED, fd, 0);
            if (mmap_ptr == MAP_FAILED) {
                perror("mmap error!");
                return -1;
            }

            // 上传阶段II 发包体
            int upload_body_try = 0;
            while (upload_body_try<3)
            {
                if(upload_body_try == 3)
                {
                    return -1;
                }
                std::cout << "\nenter upload_body while"<<"\n";
                
                //这里改成什么函数会比较好？
                //tcp_send(socket_fd.GetFd(), mmap_ptr, file_size);
                if( !socket_fd.Send((void *)&(*mmap_ptr), file_size) )
                {
                    socket_fd.Close();
                    socket_fd.Socket();
                    if(! (socket_fd.Connect(SERVER_IP_ADDR_1, SERVER_PORT)) )
                    {
                        socket_fd.Close();
                        socket_fd.Socket();
                        bool Connected_Change = socket_fd.Connect(SERVER_IP_ADDR_2, SERVER_PORT);
                        std::cout<<"clientInit to SERVER_IP_ADDR_2"<<"\n";
                        std::cout<<"Connected_Change: "<<Connected_Change<<"\n";
                    }         
                    std::cout<<"Send File Body Faild\n";
                }
                std::cout << " === SMALL_FILE BODY SENT ===\n";

                // 上传阶段II 等确认
                char buf_small_upload_body[sizeof(Package)] = {0};
                if(socket_fd.Recv(&buf_small_upload_body, sizeof(Package)) <= 0)
                {
                    std::cout << "\nBody Recv Not OK\n";
                    ++ upload_body_try;
                    continue;
                }
                else
                {
                    std::cout << "\nBody Recv OK\n";
                }
                

                auto *p = (Package*) buf_small_upload_body;
                if(p->msg_type == OK)
                {
                    Package *pp(new Package(UPLOAD_FILE_OVER, SMALL_FILE, file_name, file_size, 0));
                    if( !socket_fd.Send((void *)pp, sizeof(Package)) )
                    {
                        std::cout<<"Send File OVER Faild\n";
                        socket_fd.Close();
                        socket_fd.Socket();
                        if(! (socket_fd.Connect(SERVER_IP_ADDR_1, SERVER_PORT)) )
                        {
                            socket_fd.Close();
                            socket_fd.Socket();
                            bool Connected_Change = socket_fd.Connect(SERVER_IP_ADDR_2, SERVER_PORT);
                            std::cout<<"clientInit to SERVER_IP_ADDR_2"<<"\n";
                            std::cout<<"Connected_Change: "<<Connected_Change<<"\n";
                        }
                    }
                    ret = 0;
                    return 0; 
                }    
                //如果传包不ok就continue
                else 
                {
                    std::cout << "SMALL_FILE 上传阶段II 等确认 not ok\n";
                    ++ upload_body_try;
                    continue;
                }
            }
        }
        catch(...){
            std::cout << "SMALL_FILE UPLOAD ERROR\n";
            return -1;
        }
    }
}