#include <iostream>
#include <stdexcept>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>


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

  char msg[64] = {};

  std::cout << "Send message to server: ";
  std::cin >> msg;

  write(fd, msg, strlen(msg));

  char rbuf[64] = {};
  ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
  if (n < 0) {
    std::cerr << "Cannot read from server";
  }
  std::cout << "Server says: " << rbuf << std::endl;

  close(fd);
}
