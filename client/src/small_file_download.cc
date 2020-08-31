#include "../include/small_file.h"
#include <sys/mman.h>
#include <unistd.h>

int do_small_file_download(TcpSocket &socket_fd, char *file_name, u_int64_t file_size, int &ret) {
    int download_smallfile_try = 0;
    while(true){
        try{
            int file_fd = open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0777);
            truncate64(file_name, file_size);

            char *mmap_ptr = (char *)mmap(nullptr, file_size, PROT_WRITE | PROT_EXEC, MAP_SHARED, file_fd, 0);
            if ((void *)mmap_ptr == MAP_FAILED) {
                perror("mmap error!");
                return -1;
            }
            
            std::cout << "Start recv small file block\n";
            
            ssize_t recv_size=0;
            auto file_size_judge = file_size;
            while (file_size_judge > 0) {
                recv_size = socket_fd.Recv(mmap_ptr, file_size);
                if (recv_size <= 0) {
                    std::cout << "Recv File Error!\n";
                    break;
                }
                file_size_judge -= recv_size;
                mmap_ptr += recv_size;
            }

            // if (!CHECK_RET(recv_size, "Recv error! please check small_file_downlaod!"))
            // {
            //     std::cout << "Recv error! please check small_file_downlaod\n";
            //     continue;  //收包尺寸不对
            // }
            
            if (file_size_judge == 0)
            {
                std::cout << "Recv success\n";
                Package package(DOWNLOAD_FILE_OVER, SMALL_FILE, file_name, file_size, 0, 0);
                socket_fd.Send(&package, sizeof(Package));
                socket_fd.Close();
                ret = 0;
                return 0;
            }

            ++download_smallfile_try;
            if(download_smallfile_try == 1) // 3
            {
                throw 1;
            }
        }
        catch(...){
            std::cout << "SMALL_FILE DOWNLOAD ERROR, RECONNECT \n";
            socket_fd.Close();
            return -1;
        }
    }
}
