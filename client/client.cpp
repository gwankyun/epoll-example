#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <bits/pthreadtypes.h>
#include <cerrno>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
using std::string;
using std::cout;
using std::endl;

int main(int argc, char *argv[])
{
	auto ip = argv[1];
	auto port = argv[2];
	int result = 0;
	int listenFD = socket(PF_INET, SOCK_STREAM, 0);
	if (listenFD == -1)
	{
		cout << "socket:" << strerror(errno) << endl;
		return -1;
	}

	sockaddr_in address;
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
	address.sin_port = htons(atoi(port));

	epoll_event events[1024];
	int epollFD = epoll_create1(0);
	if (epollFD == -1)
	{
		cout << "epoll_create1:" << strerror(errno) << endl;
		return -1;
	}

	result = connect(listenFD, (sockaddr*)&address, sizeof(address));
	if (result == -1)
	{
		cout << "connect:" << strerror(errno) << endl;
		return -1;
	}

	string message("client");
	result = send(listenFD, message.c_str(), message.size(), 0);

	int oldOption = fcntl(listenFD, F_GETFL);
	int newOption = oldOption | O_NONBLOCK;
	fcntl(listenFD, F_SETFL, newOption);

	epoll_event event;
	event.data.fd = listenFD;
	event.events = EPOLLIN;
	epoll_ctl(epollFD, EPOLL_CTL_ADD, listenFD, &event);

	while (true)
	{
		int number = epoll_wait(epollFD, events, 1024, -1);
		for (size_t i = 0; i < number; i++)
		{
			auto ev = events[i];
			int fd = ev.data.fd;
			if (fd == listenFD)
			{
				if (ev.events & EPOLLIN)
				{
					char buffer[1024];
					memset(buffer, 0, sizeof(buffer));
					result = recv(listenFD, buffer, sizeof(buffer), 0);
					cout << buffer << endl;
				}
			}
		}
	}

	return 0;
}