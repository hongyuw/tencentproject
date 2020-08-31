#pragma once
#include "../../etc/config.h"
#include "big_file.h"
#include "small_file.h"
#include "../utils/tcp_socket.hpp"

void clientInit(TcpSocket &socket_fd){
    //TcpSocket socket_fd = TcpSocket();
    //socket_fd.Socket();
    //bool Connected_Change = socket_fd.Connect(SERVER_IP_ADDR_1, SERVER_PORT);
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
    return;
}

int64_t clientGetFileSize(TcpSocket &socket_fd,char* file_name){
    
    Package package(DOWNLOAD_SEND_FILE_NAME, FILE_TYPE_INIT_STATUS, file_name, 0, 0, 0);
    while(!socket_fd.Send(&package, sizeof(Package)))
    {
        //重连
        socket_fd.Close();
        clientInit(socket_fd);
        std::cout<<"get FileSize Send Error Reconn\n";
        //socket_fd.Send(&package, sizeof(Package));
    }
    char buf[sizeof(Package)] = {0};

    int64_t file_size;
    while( socket_fd.Recv(&buf, sizeof(buf)) <= 0 )
    {
        clientInit(socket_fd);
        while( !socket_fd.Send(&package, sizeof(Package)) )
        {
            //重连
            clientInit(socket_fd);
            std::cout<<"get FileSize Recv Error  Reconn\n";
        }
    }
    auto *p = (Package*) buf;


    printf("file type: %d\n",p->file_type);

    if(p->file_size > SMALL_FILE_SIZE_MAX)
    {
        socket_fd.Close();
    }

    return p->file_size;
}
