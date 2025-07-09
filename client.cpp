#include <iostream>
#include <stdexcept>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "utils.h"


static int32_t query(const int& fd, const char *text);


int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    std::cerr << "Cannot get free socket";
    return 1;
  }

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);  // 127.0.0.1
  int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));

  if (rv) {
    std::cerr << "Cannot bind socket on current address";
    return 1;
  }

  try {
    query(fd, "Hello1");
    query(fd, "Hello2");
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  close(fd);
}


static int32_t query(const int& fd, const char *text) {
  uint32_t len = (uint32_t)strlen(text);
  if (len > k_max_msg) {
    std::string error = "Message too long";
    throw std::runtime_error(error); 
  }

  char wbuf[4 + k_max_msg];
  memcpy(wbuf, &len, 4);
  memcpy(&wbuf[4], text, len);
 
  write_all(fd, wbuf, 4 + len);
  char rbuf[4 + k_max_msg + 1];
  errno = 0;
  read_full(fd, rbuf, 4);

  memcpy(&len, rbuf, 4);
  if (len > k_max_msg) {
    std::string error = "Message too long";
    throw std::runtime_error(error); 
  }

  read_full(fd, &rbuf[4], len);
  std::cout << "server says: " << std::string_view(&rbuf[4], len) << "\n";
  return 0;
}  
