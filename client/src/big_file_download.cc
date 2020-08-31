#include "../include/big_file.h"
#include <sys/mman.h>
#include <unistd.h>

volatile int okblock = 0;
// @description: 向上取整
inline uint64_t upper(uint64_t block_size, uint32_t page_size) {
  return (block_size % page_size) ? block_size * ((block_size / page_size) + 1)
                                  : block_size;
}

// @fuck: 图省事,先放这儿吧...
char *global_mmap_start_ptr = nullptr;
uint64_t global_block_size = 0;

void *thr_start(void *arg) {
  // arg init
  ThreadArgPtr tupPtr = *((ThreadArgPtr *)arg);
  off64_t offset;
  char *file_name;
  int32_t __fd, block_no;
  uint64_t real_block_size;
  uint64_t file_size;
  char* nptr;
  std::tie(file_name, __fd, offset, real_block_size, block_no, file_size,nptr) = *tupPtr;
  printf("real_block_size: %d  file_size:%d  block_no:%d\n",real_block_size,file_size,block_no);
  int down_connect = block_no % 2;
  while (true)
  {
    try{
    // socket init
      TcpSocket socket_fd = TcpSocket();
      socket_fd.Socket();
      if (down_connect == 0)
        socket_fd.Connect(SERVER_IP_ADDR_1, SERVER_PORT);
      else
        socket_fd.Connect(SERVER_IP_ADDR_2, SERVER_PORT);

      // send head
      std::shared_ptr<Package> package(
          new Package(DOWNLOAD_SEND_BLOCK_NUM, BIG_FILE, file_name, file_size, block_no, 0));
      if( !socket_fd.Send((void *)&(*package), sizeof(Package)) )
      {
        printf("send download block head error\n");
        socket_fd.Close();
        throw 1;
        continue;
      }

      // recv block data
      uint64_t writed_size = 0;
      char *thr_mmap_ptr = global_mmap_start_ptr + block_no * global_block_size;
      int64_t buf_size = TRANSPORT_BUFF_SIZE_EACH_TIME;
      while (true) {
        if (real_block_size == writed_size) // recv over
        {
          printf("thread %d received size %d\n",block_no,writed_size);
          ++okblock;
          printf("block %d receive complete\n",block_no);
          socket_fd.Close();
          return nullptr;
        }
	int64_t use_size = buf_size <= (real_block_size - writed_size) ? buf_size : (real_block_size - writed_size);
        ssize_t recv_size = socket_fd.Recv(thr_mmap_ptr + writed_size, use_size);
        if (recv_size <= 0)
        {
          printf("proxy has been closed\n");
          socket_fd.Close();
          throw 1;
          printf(" can't reach\n");
          break;
        }
        // msync(thr_mmap_ptr + writed_size, recv_size, MS_SYNC);
        writed_size += recv_size;
      }
    }
    catch(...){
      down_connect = abs(down_connect-1);
      continue;
    }
  }
}

void do_big_file_download(char *file_name, uint64_t file_size) {
  //初始化block，分配内存空间
  const int32_t thr_num = SERVER_MAX_THREADS;
  const uint64_t block_size = file_size / thr_num;
  const uint64_t last_block = file_size % thr_num;

  int fd = open(file_name, O_RDWR | O_CREAT | O_TRUNC, 0777);
  truncate64(file_name, file_size);
  char *mmap_ptr = (char *)mmap(nullptr, file_size, PROT_WRITE | PROT_EXEC, MAP_SHARED, fd, 0);
  if ((void *)mmap_ptr == MAP_FAILED) {
    perror("mmap error!");
    return;
  }

  // TODO: kill global varibles
  global_block_size = block_size;
  global_mmap_start_ptr = mmap_ptr;

  pthread_t *tid = new pthread_t[thr_num];
  std::vector<ThreadArgPtr> vec(thr_num);

  for (int32_t i = 0; i < thr_num; ++i) {
    off64_t offset = i * block_size;
    uint64_t real_block_size =
        (i == thr_num - 1) ? (block_size + last_block) : block_size;                    //最后一块合并到前一块
    ThreadArgPtr arg =
        ThreadArgPtr(new ThreadArg(file_name, -1, offset, real_block_size, i, file_size, nullptr));
    vec[i] = arg;

    int32_t res =
        pthread_create(&tid[i], nullptr, thr_start, (void *)&(vec[i]));

    if (res != 0) {
      std::cerr << "create thr error: " << i-- << std::endl;
      continue;
    }
  }

  // destroy
  for (int32_t i = 0; i < thr_num; ++i) {
    pthread_join(tid[i], nullptr);
  }

  printf("all bigfile thread joined\n");
  //回finish包
  // socket init
  if(okblock==thr_num){
    TcpSocket socket_fd = TcpSocket();
    socket_fd.Socket();
    if(!socket_fd.Connect(SERVER_IP_ADDR_1, SERVER_PORT))
    {
      socket_fd.Connect(SERVER_IP_ADDR_2, SERVER_PORT);
    }
    // send head
    std::shared_ptr<Package> package(
        new Package(DOWNLOAD_FILE_OVER, BIG_FILE, file_name, file_size, 0, 0));
    while(!socket_fd.Send((void *)&(*package), sizeof(Package)))
    {
      if(!socket_fd.Connect(SERVER_IP_ADDR_1, SERVER_PORT))
      {
        socket_fd.Connect(SERVER_IP_ADDR_2, SERVER_PORT);
      }
      //socket_fd.Send((void *)&(*package), sizeof(Package));
    }
    printf("receive complete, okblock = %d/%d\n",okblock,thr_num);
  }
  else
  {
    printf("receive error, okblock = %d/%d\n",okblock,thr_num);
    return;
  }
  
  // if (munmap(mmap_ptr, upper(file_size, sysconf(_SC_PAGESIZE))) == -1) {
  //   std::cerr << "munmap error!\n";
  // }
  
  if ( munmap(mmap_ptr,file_size) == -1) {
    std::cerr << "munmap error!\n";
  }
  printf("fd:%d\n",fd);
  close(fd);
}
