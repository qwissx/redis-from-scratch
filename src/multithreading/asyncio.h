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


struct Response {
  uint32_t status = 0;
  std::string *data;
};

void fd_set_nonblock(const int& fd);
void prepare_poll_args(
  const int& fd,
  std::vector<Conn*>& fd2conn, 
  std::vector<pollfd>& poll_args
);
void wait_for_read(std::vector<pollfd>& poll_args);
void handle_listening_socket(
  const int& fd,
  std::vector<Conn*>& fd2conn,
  const std::vector<pollfd>& poll_args
); 
void handle_connection_socket(
  std::vector<Conn*>& fd2conn, const std::vector<pollfd>& poll_args
);
Conn* handle_accept(const int& fd);
bool try_one_request(Conn *conn);
void handle_read(Conn *conn);
void handle_write(Conn *conn);
void make_response(const Response &resp, std::vector<uint8_t> &out);
void do_request(std::vector<std::string> &cmd, Response &out);
bool read_u32(const uint8_t *&cur, const uint8_t *end, uint32_t &out);
bool read_str(const uint8_t *&cur, const uint8_t *end, size_t n, std::string &out);
int32_t parse_req(const uint8_t *data, size_t size, std::vector<std::string> &out);
  
//namespace async
}
