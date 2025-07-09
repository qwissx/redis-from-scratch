#include <iostream>
#include <stdexcept>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>


static int get_sock_file_descriptor(const int& domain, const int& type, const int& protocol=0);
static int accept_client_connection(const int& file_descriptor);
static void set_sock_options(
  const int& file_descriptor, const int& level, const int& optional_name, const int optional_value
);
static void bind_sock_address(
  const int& file_descriptor, const int& family, const int& port, const int& ip
);
static void bind_sock_listen(const int& file_descriptor, const int& backlog);
static void read_client_input(const int& connfd);


int main() {
  int fd, connfd;

  fd = get_sock_file_descriptor(AF_INET, SOCK_STREAM);
  set_sock_options(fd, SOL_SOCKET, SO_REUSEADDR, 1);

  try {
    bind_sock_address(fd, AF_INET, 1234, 0);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  try {
    bind_sock_listen(fd, SOMAXCONN);
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  while (true) {
    try {
      connfd = accept_client_connection(fd);
      read_client_input(connfd);
    } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << std::endl;
      continue;
    }
    close(connfd);
  }

  return 0;
}


static int get_sock_file_descriptor(const int& domain, const int& type, const int& protocol) {
  return socket(domain, type, protocol);
}

static int accept_client_connection(const int& file_descriptor) {
  sockaddr_in client_addr = {};
  socklen_t addrlen = sizeof(client_addr);
  int connfd = accept(file_descriptor, (sockaddr *)& client_addr, &addrlen);

  if (connfd < 0) {
    std::string error = "Failed to accept client connection";
    throw std::runtime_error(error);
  }

  return connfd;
}

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

static void read_client_input(const int& connfd) {
  char rbuf[64] = {}, wbuf[64] = {};
  ssize_t n = read(connfd, rbuf, sizeof(rbuf)-1);

  if (n < 0) {
    std::string error = "Cannot read client input";
    throw std::runtime_error(error);
  }

  std::cout << "Client says: " << rbuf << std::endl;
  std::cout << "Send him message: ";
  std::cin >> wbuf;
  
  write(connfd, wbuf, strlen(wbuf));
}
