inline constexpr size_t k_max_msg = 4096; 

int32_t read_full(const int& fd, char *buf, size_t n);
int32_t write_all(const int& fd, const char *buf, size_t n);
void signal_handler(const int& signum);
