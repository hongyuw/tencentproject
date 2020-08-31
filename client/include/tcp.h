#include <cerrno>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string>
#include "../../etc/config.h"

int tcp_receive(int client_fd, char *buf, int64_t length);

int tcp_send(int sock_fd, char *buffer, int64_t length);

int tcp_init();
int tcp_init_md5();

int tcp_connect(int fd, const std::string &ip, const uint16_t port);

/*
  SDBMHash function to calculate send to which disk(server)
  @param str  input filepath
  @return num
*/
// unsigned int my_hash(char *str);

