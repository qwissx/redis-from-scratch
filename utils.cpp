#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cassert>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "utils.h"


int32_t read_full(const int& fd, char *buf, size_t n) {
  while (n > 0) {
    ssize_t rv = read(fd, buf, n);

    if (rv < 0) {
      std::string error = "Cannot read client input";
      throw std::runtime_error(error);
    }

    assert((size_t)rv <= n);
    n -= (size_t)rv;
    buf += rv;
  }
  return 0;
}

int32_t write_all(const int& fd, const char *buf, size_t n) {
  while (n > 0) {
    ssize_t rv = write(fd, buf, n);

    if (rv < 0) {
      std::string error = "Cannot write server input";
      throw std::runtime_error(error);
    }

    assert((size_t)rv <= n);
    n -= (size_t)rv;
    buf += rv;
  }
  return 0;
}

