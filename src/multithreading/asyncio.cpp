#include <iostream>
#include <fcntl.h>
#include <stdexcept>
#include <cstring>
#include <vector>
#include <poll.h>
#include <cstdint>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cassert>
#include <map>

#include "asyncio.h"
#include "../utils/utils.h"

static std::map<std::string, std::string> g_data;

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

Conn* handle_accept(const int& fd) {
  sockaddr_in client_addr = {};
  socklen_t addrlen = sizeof(client_addr);
  int connfd = accept(fd, (sockaddr*)&client_addr, &addrlen);
  if (connfd < 0) {
    std::string error = "Cannot accept connection";
    throw std::runtime_error(error);
  }

  fd_set_nonblock(connfd);
  Conn *conn = new Conn();
  conn->fd = connfd;
  conn->want_read = true;
  return conn;
}

static void buf_consume(std::vector<uint8_t> &buf, size_t n) {
  buf.erase(buf.begin(), buf.begin() + n);
}

static void buf_append(std::vector<uint8_t> &buf, const uint8_t *data, size_t n) {
  buf.insert(buf.end(), data, data + n);
}

void make_response(const Response &resp, std::vector<uint8_t> &out) {
  uint32_t resp_len = 4 + (uint32_t)resp.data->size();

  buf_append(out, (const uint8_t *)&resp_len, 4);
  buf_append(out, (const uint8_t *)&resp.status, 4);
  buf_append(out, (const uint8_t *)resp.data->data(), resp.data->size());
}

void do_request(std::vector<std::string> &cmd, Response &out) {
  if (cmd.size() == 2 && cmd[0] == "get") {
    auto it = g_data.find(cmd[1]);
    if (it == g_data.end()) {
      out.status = ResponseStatus::RES_NX;
      return;
    }
    out.data = &it->second;
  } else if (cmd.size() == 3 && cmd[0] == "set") {
    g_data[cmd[1]].swap(cmd[2]);
  } else if (cmd.size() == 2 && cmd[0] == "del") {
    g_data.erase(cmd[1]);
  } else {
    out.status = ResponseStatus::RES_ERR;
  }
}

bool read_u32(const uint8_t *&cur, const uint8_t *end, uint32_t &out) {
    if (cur + 4 > end) {
        return false;
    }
    memcpy(&out, cur, 4);
    cur += 4;
    return true;
}

bool read_str(const uint8_t *&cur, const uint8_t *end, size_t n, std::string &out) {
    if (cur + n > end) {
        return false;
    }
    out.assign(cur, cur + n);
    cur += n;
    return true;
}

int32_t parse_req(const uint8_t *data, size_t size, std::vector<std::string> &out) {
    const uint8_t *end = data + size;
    uint32_t nstr = 0;
    if (!read_u32(data, end, nstr)) {
        return -1;
    }
    if (nstr > k_max_msg) {
        return -1;  // safety limit
    }

    while (out.size() < nstr) {
        uint32_t len = 0;
        if (!read_u32(data, end, len)) {
            return -1;
        }
        out.push_back(std::string());
        if (!read_str(data, end, len, out.back())) {
            return -1;
        }
    }
    if (data != end) {
        return -1;  // trailing garbage
    }
    return 0;
}

bool try_one_request(Conn *conn) {
  if (conn->incoming.size() < 4) {
    return false;
  }
  uint32_t len = 0;
  memcpy(&len, conn->incoming.data(), 4);
  if (len > k_max_msg) {
    conn->want_close = true;
    return false;
  }

  if (4 + len > conn->incoming.size()) {
    return false;
  }

  const uint8_t *request = &conn->incoming[4];

  std::vector<std::string> cmd;
  if (parse_req(request, len, cmd) < 0) {
      conn->want_close = true;
      return false;   // error
  }
  Response resp;
  do_request(cmd, resp);
  make_response(resp, conn->outgoing);
  
  buf_consume(conn->incoming, 4+len);
  return true;
}

void handle_write(Conn *conn) {
  assert(conn->outgoing.size() > 0);
  ssize_t rv = write(conn->fd, conn->outgoing.data(), conn->outgoing.size());
  if (rv < 0 and EAGAIN) return;

  if (rv < 0) {
    conn->want_close = true;
    std::cerr << "Error: Cannot write to file descriptor" << std::endl;
    return;
  }

  buf_consume(conn->outgoing, (size_t)rv);

  if (conn->outgoing.size() == 0) {
    conn->want_read = true;
    conn->want_write = false;
  }
} 

void handle_read(Conn *conn) {
  uint8_t buf[64 * 1024];
  ssize_t rv = read(conn->fd, buf, sizeof(buf));
  if (rv < 0 and errno == EAGAIN) return;

  if (rv < 0) {
    conn->want_close = true;
    std::cerr << "Error: Cannot read file descriptor" << std::endl;
    return;
  }

  if (rv == 0) {
    if (conn->incoming.size() == 0) {
      std::cout << "Client closed" << std::endl;
    } else {
      std::string error = "Unexpected end of file";
      throw std::runtime_error(error);
    }
    conn->want_close = true;
    return;
  }

  buf_append(conn->incoming, buf, (size_t)rv);
  while (try_one_request(conn)) {};

  if (conn->outgoing.size() > 0) {
    conn->want_read = false;
    conn->want_write = true;
    return handle_write(conn);
  }
}

 
//namespace async
}
