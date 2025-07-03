#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>

const int optionalValues = 1;

int main() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optionalValues, sizeof(optionalValues));

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(1234);
    addr.sin_addr.s_addr = htonl(0);
    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv) { die("bind()"); }

    return 0;
}
