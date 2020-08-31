//
// Created by Jiashuai on 2020/8/17.
//


#include "../include/jobs.h"
#include <vector>

struct job jobs[MSG_TYPE_SIZE - MSG_TYPE_INIT_STATUS - 1] = {
        ADDJOB(DOWNLOAD_SEND_FILE_NAME, job_download_send_file_info),
        ADDJOB(DOWNLOAD_SEND_BLOCK_NUM, job_download_send_block_content),
        ADDJOB(DOWNLOAD_FILE_OVER,job_download_unmap_file),
        ADDJOB(UPLOAD_SEND_FILE_INFO,job_upload_recv_file_info),
        ADDJOB(UPLOAD_SEND_BLOCK_INFO,job_upload_recv_block_content),
        ADDJOB(UPLOAD_BREAKPOINT_QUERY,job_upload_breakpoint_send),
        ADDJOB(UPLOAD_FILE_OVER,job_upload_unmap_file),
        ADDJOB(MD5_CAL, job_cal_md5),
};


extern std::map<std::string,char*>* file_name_address_map;   //用于储存文件的映射地址，文件名为key，映射addr为value

extern std::map<std::string,char*>* file_upload_md5_map;   //用于储存上传文件的md5，文件名为key，md5为value

class block_over_num {
public:
    int value;
    std::mutex __mutex;
    block_over_num(int x): value(x){}
};
int fd_download = 0;
int fd_upload = 0;
block_over_num BLOCK_OVER_NUM(0);
std::vector<off64_t> break_point_record(BLOCK_TOTAL_NUM,0); //上传时，储存对应的块数接收到了多少大小

// 让程序支持新的 job，在 jobs[MAX_JOBS] 里添加相应的函数即可！


//===========================================下载部分======================================================
/*下载：阶段I，第一个包(客户端请求文件信息)
 * 服务端发送给客户端文件信息
 * 如果是小文件，发送完文件信息后 直接 发送小文件内容
 * 如果是大文件，只发送文件信息
*/
int job_download_send_file_info(int socket_fd, Package *p) {
    std::string file_path = std::string(SERVER_FILE_SAVE_PATH) + p->file_name;
    Package rsp;
    memcpy(rsp.file_name,p->file_name,strlen(p->file_name));
    fd_download = open(file_path.c_str(), O_RDWR);
    if (fd_download < 0) {
        std::string err_data = file_path + "open failed";
        perror(err_data.c_str());
        return -1;
    }
    struct stat64 stat_;
    fstat64(fd_download, &stat_);
    rsp.file_size = stat_.st_size;
    if (file_name_address_map == nullptr) {
        file_name_address_map = new std::map<std::string,char*>();
    }
    if (file_name_address_map->count(std::string(p->file_name)) == 0) {
        void* mmap_addr = mmap(NULL, stat_.st_size, PROT_READ, MAP_SHARED, fd_download, 0);
        if (mmap_addr == MAP_FAILED) {
            std::string err_data = file_path + " mmap failed";
            perror(err_data.c_str());
            close(fd_download);
            fd_download = 0;
            return -2;
        }
        file_name_address_map->insert(std::make_pair(std::string(p->file_name),(char*)mmap_addr));
    }
    if (stat_.st_size > SMALL_FILE_SIZE_MAX) {
        rsp.file_type = BIG_FILE;
        tcp_send(socket_fd,(char*)&rsp,sizeof(rsp));
    } else {
        rsp.file_type = SMALL_FILE;
        tcp_send(socket_fd,(char*)&rsp,sizeof(rsp));
        tcp_send(socket_fd,(char*)file_name_address_map->at(std::string(p->file_name)),rsp.file_size);
    }
    //std::string md5_ ="md5sum " + file_path;
    //system(md5_.c_str());
    return 0;
}


/*下载: 阶段II，收到第二个包(客户端发送block信息)
 * 根据文件名找到文件映射地址
 * 发送对应部分文件
 * */
int job_download_send_block_content(int socket_fd, Package *p) {
    auto file_size = p->file_size;
    auto block_size = file_size / BLOCK_TOTAL_NUM;
    char* content = file_name_address_map->at(p->file_name);
    content += block_size * p->block_now_num;
    if (p->block_now_num == BLOCK_TOTAL_NUM - 1) {
        auto front_size = block_size * (p->block_now_num);
        block_size = file_size - front_size;
    }
    content += p->off_set;
    block_size -= p->off_set;
    tcp_send(socket_fd,content,block_size);
    return 0;
}

/*下载: 阶段III, 收到第三个包(客户端收完全部数据，会回复一个DOWNLOAD_FILE_OVER消息), 可以带偏移量
 * 关闭对已打开文件的映射，并从hashmap中删除
 * */
int job_download_unmap_file(int socket_fd, Package *p) {

    void* mmap = file_name_address_map->at(p->file_name);
    munmap(mmap,p->file_size);
    //close(fd_download);
    fd_download = 0;
    auto to_delete = file_name_address_map->find(p->file_name);
    file_name_address_map->erase(to_delete);
    return 0;
    /*
    auto file_size = p->file_size;
    auto block_size = file_size / BLOCK_TOTAL_NUM;
    char* content = file_name_address_map->at(p->file_name);
    if (p->block_now_num == BLOCK_TOTAL_NUM - 1) {
        block_size = file_size % BLOCK_TOTAL_NUM;
    }
    content += p->off_set;
    block_size -= p->off_set;
    tcp_send(socket_fd,content,block_size);
    return 0;
     */
}


//===========================================上传部分======================================================

/*上传: 阶段I   服务器接收客户端的文件信息，以创建对应的文件*/
int job_upload_recv_file_info(int socket_fd, Package *p) {
    std::string file_path = std::string(SERVER_FILE_SAVE_PATH) + p->file_name;
    fd_upload = open(file_path.c_str(), O_RDWR| O_CREAT);
    if (fd_upload < 0) {
        std::string err_data = file_path + " : create failed";
        perror(err_data.c_str());
        return -1;
    }

    ftruncate(fd_upload,p->file_size);
    if (file_name_address_map == nullptr) {
        file_name_address_map = new std::map<std::string,char*>();
    }
    if (file_name_address_map->count(std::string(p->file_name)) == 0) {
        void* mmap_addr = mmap(NULL, p->file_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd_upload, 0);
        if (mmap_addr == MAP_FAILED) {
            std::string err_data = file_path + " mmap failed";
            perror(err_data.c_str());
            close(fd_upload);
            fd_upload = 0;
            return -2;
        }
        file_name_address_map->insert(std::make_pair(std::string(p->file_name),(char*)mmap_addr));
    }
    if (p->file_type == SMALL_FILE) {
        p->msg_type = OK;
        tcp_send(socket_fd,(char*)p,strlen((char*)p));  //发回包，告知已准备好接收小文件主体内容
        uint64_t len = p->file_size;
        char* opt = file_name_address_map->at(std::string(p->file_name));
        while (len > 0) {
            int rec_size = tcp_receive(socket_fd,opt,len);
            if (rec_size < 0) return -1;
            len -= rec_size;
            opt += rec_size;
        }

        if (len == 0) { //如果接收完毕，告知客户端UPLOAD_BLOCK_OVER 否则 UPLOAD_BLOCK_FAILED
            p->msg_type = OK;
            tcp_send(socket_fd, (char*)p, strlen((char*)p));
        } else {
            p->msg_type = FAILED;
            tcp_send(socket_fd, (char*)p, strlen((char*)p));
        }
        //system("md5sum /data/group1/client.10K");
    } else {
        p->msg_type = OK;
        tcp_send(socket_fd,(char*)p,strlen((char*)p));      //大文件只返回 READY
    }
    return 0;
}

/*上传: 阶段II  服务器接收大文件主体内容
 * */
int job_upload_recv_block_content(int socket_fd, Package *p) {
    auto file_size = p->file_size;
    auto block_size = file_size / BLOCK_TOTAL_NUM;
    char* content = file_name_address_map->at(p->file_name);

    content += block_size * p->block_now_num;
    if (p->block_now_num == BLOCK_TOTAL_NUM - 1) {
        auto front_size = block_size * (p->block_now_num);
        block_size = file_size - front_size;
    }
    content += p->off_set;
    block_size -= p->off_set;
    //先回包，告知已收到
    p->msg_type = OK;
    tcp_send(socket_fd, (char*)p, strlen((char*)p));
    int rec_size = 0;
    //获取信息
    int64_t buf_size = TRANSPORT_BUFF_SIZE_EACH_TIME;
    while (block_size > 0) {
        break_point_record[p->block_now_num] += rec_size;
        int64_t use_size = buf_size <= block_size ? buf_size : block_size;
        rec_size = tcp_receive(socket_fd, content, use_size);//p->file_size);
        if (rec_size < 0) return -1;
        block_size -= rec_size;
        content += rec_size;
    }
    if (block_size == 0) {
        //--取消--加锁判断是否已经所有的block全部收完了
        //BLOCK_OVER_NUM.__mutex.lock();
        //BLOCK_OVER_NUM.value++;
        /*
        if (BLOCK_OVER_NUM.value == BLOCK_TOTAL_NUM) {  //如果收完了，就回复一个UPLOAD_FILE_OVER，并且关闭所有文件及映射，并重置fd
            munmap(file_name_address_map->at(p->file_name),p->file_size);
            close(fd_upload);

            fd_upload = 0;
            BLOCK_OVER_NUM.value = 0;
            for (int i = 0; i < BLOCK_TOTAL_NUM; i++) break_point_record[i] = 0;
            auto to_delete = file_name_address_map->find(p->file_name);
            file_name_address_map->erase(to_delete);
            p->msg_type=UPLOAD_FILE_OVER;
            tcp_send(socket_fd,(char*)p,strlen((char*)p));
        } else {    //如果没有全部收完毕，就回复一个OK
         */
        p->msg_type = OK;
        tcp_send(socket_fd,(char*)p,strlen((char*)p));

        //BLOCK_OVER_NUM.__mutex.unlock();
    } else {    //如果接收失败，就回复FILED
        p->msg_type = FAILED;
        tcp_send(socket_fd,(char*)p,strlen((char*)p));
    }

    return 0;
}

/*上传: 断点续传,断点查询
 *
 * */
int job_upload_breakpoint_send(int socket_fd, Package *p) {
    int block_num = p->block_now_num;
    p->off_set = break_point_record[block_num];
    tcp_send(socket_fd,(char*)p,strlen((char*)p));
}

/*上传完毕：客户端告知服务器上传完毕，服务器解除文件映射
 * */
int job_upload_unmap_file(int socket_fd, Package* p) {
    /*p->msg_type = FAILED;
    if (file_upload_md5_map == nullptr) {
        file_upload_md5_map = new std::map<std::string,char*>();
    }
    if (file_upload_md5_map->count(std::string(p->file_name)) != 0) {
        char* md5 =file_upload_md5_map->at(std::string(p->file_name));
        memcpy(p->md5, md5, strlen(md5));
        p->msg_type = OK;
        tcp_send(socket_fd,(char*)p,strlen((char*)p));
    }


    std::string file_path = std::string(SERVER_FILE_SAVE_PATH) + p->file_name;
    if (fd_upload == 0) {
        fd_upload = open(file_path.c_str(),O_RDONLY);
        if (fd_upload <= 0) {
            printf("upload file over -> open %s failed!\n",file_path.c_str());
            tcp_send(socket_fd,(char*)p,strlen((char*)p));
            return -1;
        }
    }

    if (file_upload_md5_map->count(std::string(p->file_name)) == 0) {
        void* mmap_addr = nullptr;
        if (file_name_address_map->count(std::string(p->file_name)) == 0) {
            mmap_addr = mmap(NULL, p->file_size, PROT_READ, MAP_SHARED, fd_upload, 0);
            if (mmap_addr == MAP_FAILED) {
                std::string err_data = "upload file over -> " + file_path + " mmap failed";
                perror(err_data.c_str());
                close(fd_download);
                fd_download = 0;
                tcp_send(socket_fd,(char*)p,strlen((char*)p));
                return -2;
            }
        } else {
            mmap_addr = file_name_address_map->at(std::string(p->file_name));
        }

        file_upload_md5_map->insert(std::make_pair(std::string(p->file_name),(char*)mmap_addr));
    }
    if ()
    MD5 md5;
    md5.update(mmap,p->file_size);
    memcpy(p->md5, md5.toString().c_str(), md5.toString().size());
    tcp_send(socket_fd,(char*)p,strlen((char*)p));
    printf("md5(%s): %s\n",p->file_name,md5.toString().c_str());
    munmap(mmap,p->file_size);
    close(fd_download);
    fd_upload = 0;
    auto to_delete = file_name_address_map->find(p->file_name);
    file_name_address_map->erase(to_delete);*/
    return 0;
}


/*
 * 计算文件md5sum
 * */
int job_cal_md5(int socket_fd, Package *p) {
    std::string md5_opt = "md5sum " + std::string(SERVER_FILE_SAVE_PATH) + p->file_name;
    system(md5_opt.c_str());
    //MD5 md5;
    //void * mmap_add = file_name_address_map->at(p->file_name);
    //md5.update(mmap_add,p->file_size);
    //printf("md5[%s]: %s\n",p->file_name,md5.toString().c_str());
    p->msg_type = OK;
    tcp_send(socket_fd,(char*)p,strlen((char*)p));
    return 0;
}
