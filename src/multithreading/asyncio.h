#include <vector>
#include <poll.h>

namespace async {
struct Conn {
  int fd = -1;
  bool want_read = false;
  bool want_write = false;
  bool want_close = false;
  std::vector<uint8_t> incoming;
  std::vector<uint8_t> outgoing;
};

void fd_set_nonblock(const int& fd);
void prepare_poll_args(
  const int& fd,
  std::vector<Conn*>& fd2conn, 
  std::vector<pollfd>& poll_args
);
void wait_for_read(const std::vector<pollfd>& poll_args);
void handle_listening_socket(
  const int& fd,
  std::vector<Conn*>& fd2conn,
  const std::vector<pollfd>& poll_args
); 
void handle_connection_socket(
  std::vector<Conn*>& fd2conn, const std::vector<pollfd>& poll_args
);
  
//namespace async
}
