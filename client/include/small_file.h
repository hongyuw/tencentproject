#include "../../etc/config.h"
#include "../utils/tcp_socket.hpp"
#include <memory>
#include <stdint.h>
#include <tuple>


Package *
set_package(MSG_TYPE msg_type, char filename[256], size_t block_len, unsigned int disk_no);

char *split_filename(char *filename);

int do_small_file_upload_new(char *file_name, size_t file_size);

int do_small_file_download(TcpSocket &socket_fd, char *file_name, u_int64_t file_size, int &ret);