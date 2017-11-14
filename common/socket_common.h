#ifndef SOCKET_COMMON
#define SOCKET_COMMON
#include <string>
int setNonblocking(int fd)
{
    int oldOption = fcntl(fd, F_GETFL);
    int newOption = oldOption | O_NONBLOCK;
    fcntl(fd, F_SETFL, newOption);
    return oldOption;
}

void epollAdd(int epollFD, int fd, uint32_t events)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = events;
    epoll_ctl(epollFD, EPOLL_CTL_ADD, fd, &event);
}

void epollMod(int epollFD, int fd, uint32_t events)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = events;
    epoll_ctl(epollFD, EPOLL_CTL_MOD, fd, &event);
}

sockaddr_in makeSockaddr_in(std::string ip, int port)
{
    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
    address.sin_port = htons(uint16_t(port));
    return address;
}
#endif // !SOCKET_COMMON
