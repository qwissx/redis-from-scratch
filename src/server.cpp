#include <iostream>
#include <vector>
#include <stdexcept>
#include <cstring>
#include <cassert>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <poll.h>

#include "utils/utils.h"
#include "multithreading/asyncio.h"


static int get_sock_file_descriptor(const int& domain, const int& type, const int& protocol=0);
//static int accept_client_connection(const int& file_descriptor);
static void set_sock_options(
  const int& file_descriptor, const int& level, const int& optional_name, const int optional_value
);
static void bind_sock_address(
  const int& file_descriptor, const int& family, const int& port, const int& ip
);
static void bind_sock_listen(const int& file_descriptor, const int& backlog);
//static int32_t one_request(const int& fd);


int main() {
  int fd;
  std::vector<async::Conn *> fd2conn;
  std::vector<pollfd> poll_args;

  fd = get_sock_file_descriptor(AF_INET, SOCK_STREAM);
  set_sock_options(fd, SOL_SOCKET, SO_REUSEADDR, 1);

  try {
    bind_sock_address(fd, AF_INET, 1234, 0);
    bind_sock_listen(fd, SOMAXCONN);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

 while (true) {
    try {
      async::prepare_poll_args(fd, fd2conn, poll_args);
      async::wait_for_read(poll_args);
      async::handle_listening_socket(fd, fd2conn, poll_args);
      async::handle_connection_socket(fd2conn, poll_args);
    } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << std::endl;
      return 1;
    }
  } 

  // while (true) {
  //   try {
  //     connfd = accept_client_connection(fd);
  //   } catch (const std::exception& e) {
  //     std::cerr << "Error: " << e.what() << std::endl;
  //     continue;
  //   }
  //
  //   while (true) {
  //     int32_t err = one_request(connfd);
  //     if (err) {
  //       break;
  //     } 
  //   }
  //   close(connfd);
  // }

  return 0;
}


static int get_sock_file_descriptor(const int& domain, const int& type, const int& protocol) {
  return socket(domain, type, protocol);
}

// static int accept_client_connection(const int& file_descriptor) {
//   sockaddr_in client_addr = {};
//   socklen_t addrlen = sizeof(client_addr);
//   int connfd = accept(file_descriptor, (sockaddr *)& client_addr, &addrlen);
//
//   if (connfd < 0) {
//     std::string error = "Failed to accept client connection";
//     throw std::runtime_error(error);
//   }
//
//   return connfd;
// }

static void set_sock_options(
  const int& file_descriptor, const int& level, const int& optional_name, const int optional_value
) {
  setsockopt(file_descriptor, level, optional_name, &optional_value, sizeof(optional_value));
}

static void bind_sock_address(
  const int& file_descriptor, const int& family, const int& port, const int& ip
) {
  sockaddr_in addr = {};
  addr.sin_family = family;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = htonl(ip);
  int rv = bind(file_descriptor, (const sockaddr *)&addr, sizeof(addr));
   
  if (rv) {
    std::string error = "Cannot bind socket address on " + std::to_string(ip) + ":" + std::to_string(port) + ".";
    throw std::runtime_error(error);
  }
}

static void bind_sock_listen(const int& file_descriptor, const int& backlog) {
  int rv = listen(file_descriptor, backlog);

  if (rv) {
    std::string error = "Cannot listen on socket";
    throw std::runtime_error(error);
  }
  std::cout << "Server is ready to accept connection on 0.0.0.0:1234" << std::endl;

}

// static int32_t one_request(const int& connfd) {
//   char rbuf[4 + k_max_msg];
//   errno = 0;
//   int32_t err = -1;
//
//   try {
//   err = read_full(connfd, rbuf, 4);
//   uint32_t len = 0;
//   memcpy(&len, rbuf, 4);
//
//   if (len > k_max_msg) {
//     std::cerr << "Error: message from client to long" << std::endl;
//     return -1;
//   }
//
//   err = read_full(connfd, &rbuf[4], len);
//   std::cout << "Client says: " << std::string_view(&rbuf[4], len) << std::endl;
//
//   const char reply[] = "world";
//   char wbuf[4 + sizeof(reply)];
//
//   len = (uint32_t)strlen(reply);
//
//   memcpy(wbuf, &len, 4);
//   memcpy(&wbuf[4], reply, len);
//
//   return write_all(connfd, wbuf, 4 + len);
//   } catch (const std::exception& e) {
//     std::cerr << "Error: " << e.what() << std::endl;
//     return err;
//   }
// }
