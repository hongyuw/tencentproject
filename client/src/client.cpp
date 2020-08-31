#include "../include/clienthandler.h"
#include "../include/big_file.h"
#include "../include/small_file.h"
#include "../utils/tcp_socket.hpp"

int time_out = 0;

int main(int argc, char* argv[]){   
    //检查输入合法性，输入格式举例：./client download filename
    if(argc!=4){
        printf("input argument incorrect\n");
        return 0;
    }
    clock_t startTime,endTime;
    //startTime = clock();

    //初始化client，连接两个proxy
    TcpSocket socket_fd;
    socket_fd.Socket();
    clientInit(socket_fd);

    //根据输入的jobname决定执行上传还是下载
    const char *jobname = argv[1];
    const char *filename = argv[2];
    const char *time_out_type = argv[3];
    if (strcmp(time_out_type, "kx") == 0){
        time_out = 1;
        printf("time_out: %d\n", time_out);
    }
    else{
        time_out = 10;
        printf("time_out: %d\n", time_out);
    }

    if(strcmp(jobname,"download")==0){
        //get file size
        uint64_t file_size = clientGetFileSize(socket_fd,(char*)filename);
        printf("download size: %d\n",file_size);
        if(file_size > SMALL_FILE_SIZE_MAX){
            do_big_file_download((char *)filename, file_size);
        }
        else
        {
            int ret = -1;
            do_small_file_download(socket_fd, (char*)filename, file_size, ret);
            while(ret != 0)
            {
                clientInit(socket_fd);
                std::cout<<"download init suc";
                uint64_t file_size = clientGetFileSize(socket_fd,(char*)filename);
                do_small_file_download(socket_fd, (char*)filename, file_size, ret);
            }
        }
        
    }

    else if(strcmp(jobname,"upload")==0){
        //获取file_size，file_fd,filename    
        printf("enter upload\n");

        std::string file_path = std::string(filename);

        int file_fd = open(file_path.c_str(), O_RDONLY, 0777);

        if (file_fd < 0) {
            std::string err_data = "open upload file failed";
            perror(err_data.c_str());
            file_fd = open(file_path.c_str(), O_RDONLY, 0777);
        }
        std::cout << "open upload file success\n";

        struct stat64 stat_;
        fstat64(file_fd, &stat_);
        long file_size = stat_.st_size;
        std::cout<<"upload file size: "<<file_size<<"\n";
        close(file_fd);

        if(file_size > SMALL_FILE_SIZE_MAX){
            do_big_file_upload_new(filename, file_size);
        }
        else
        {
            do_small_file_upload_new((char *)filename, file_size);
        }
    }
    
    else
    {
        printf("argument input error\n");
        return 0;
    }
    //endTime = clock();
    //std::cout << "Totle Time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
    return 0;
}
