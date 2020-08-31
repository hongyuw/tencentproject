#pragma once

#include "../../etc/config.h"
#include "../utils//tcp_socket.hpp"
#include <memory>
#include <stdint.h>
#include <tuple>
#include <vector>
#include <pthread.h>
#include "../include/tcp.h"

#include <sys/mman.h>

// <file_name, fd, offset, real_block_size, block_no,file_size>
using ThreadArg = std::tuple<char *, int32_t, off64_t, uint64_t, uint32_t,uint64_t,char *>;
using ThreadArgPtr = std::shared_ptr<ThreadArg>;

void do_big_file_upload(int32_t fd, const char *file_name, const uint64_t file_size);

void do_big_file_upload_new(const char *file_name, const uint64_t file_size);

void do_big_file_download(char *file_name, uint64_t file_size);
