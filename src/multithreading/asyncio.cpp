#include <fcntl.h>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <poll.h>
#include <cstdint>
#include <unistd.h>

#include "asyncio.h"


namespace async {
void fd_set_nonblock(const int& fd) {
  int flags = fcntl(fd, F_GETFL, 0);

  if (flags < 0) {
    std::string error = "Cannot get socket flags";
    throw std::runtime_error(error);
  }
  flags |= O_NONBLOCK;
  fcntl(fd, F_SETFL, flags);

  if (flags < 0) {
    std::string error = "Cannot set socket mode to non block";
    throw std::runtime_error(error);
  }
}

void prepare_poll_args(const int& fd, std::vector<Conn*>& fd2conn, std::vector<pollfd>& poll_args) {
  poll_args.clear();
  pollfd pfd = {fd, POLLIN, 0};
  poll_args.push_back(pfd);

  for (Conn *conn : fd2conn) {
    if (!conn) {
      continue;
    }

    pollfd pfd = {conn->fd, POLLERR, 0};

    if (conn->want_read) {
      pfd.events |= POLLIN;
    }
    if (conn->want_write) {
      pfd.events |= POLLOUT;
    }
    poll_args.push_back(pfd);
  }
}

void wait_for_read(std::vector<pollfd>& poll_args) {
  int rv = poll(poll_args.data(), (nfds_t)poll_args.size(), -1);
  if (rv < 0 and errno == EINTR) {
    return;
  }
  if (rv < 0) {
    std::string error = "Cannot poll events";
    throw std::runtime_error(error);
  }
}

void handle_listening_socket(const int& fd, std::vector<async::Conn*>& fd2conn, const std::vector<pollfd>& poll_args) {
  if (poll_args[0].revents) {
    if (Conn *conn = handle_accept(fd)) {
      if (fd2conn.size() <= (size_t)conn->fd) fd2conn.resize(conn->fd+1);
      fd2conn[conn->fd] = conn;
    }
  }
}

void handle_connection_socket(
  std::vector<Conn*>& fd2conn, const std::vector<pollfd>& poll_args
) {
  for (size_t i = 1; i < poll_args.size(); ++i) {
    uint32_t ready = poll_args[i].revents;
    Conn *conn = fd2conn[poll_args[i].fd];
    if (ready & POLLIN) {
      handle_read(conn);
    }
    if (ready & POLLOUT) {
      handle_write(conn);
    }

    if ((ready & POLLERR) or conn->want_close) {
      (void)close(conn->fd);
      fd2conn[conn->fd] = NULL;
      delete conn;
    }
  }
}
 
//namespace async
}
