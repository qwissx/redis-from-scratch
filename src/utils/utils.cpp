#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cassert>
#include <netinet/in.h>
#include <unistd.h>
#include <csignal>
#include <sys/socket.h>
#include <sys/types.h>

#include "utils.h"


volatile sig_atomic_t signal_recevied = 0;

void signal_handler(int signum) {
  if (signum == SIGTERM) {
    std::cout << "Stopping process. PID: " << getpid() << std::endl;
    signal_recevied = 1;
    exit(signum);
  }
}

int32_t read_full(const int& fd, char *buf, size_t n) {
  signal(SIGTERM, signal_handler);

  while (n > 0) {
    if (signal_recevied) {
      std::string error = "Stopping during 15 signal...";
      throw std::runtime_error(error);
    }

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

