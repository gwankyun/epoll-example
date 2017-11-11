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

	result = bind(listenFD, (sockaddr*)&address, sizeof(address));
	if (result == -1)
	{
		cout << "bind:" << strerror(errno) << endl;
		return -1;
	}

	epoll_event events[1024];
	int epollFD = epoll_create1(0);
	if (epollFD == -1)
	{
		cout << "epoll_create1:" << strerror(errno) << endl;
		return -1;
	}

	int oldOption = fcntl(listenFD, F_GETFL);
	int newOption = oldOption | O_NONBLOCK;
	fcntl(listenFD, F_SETFL, newOption);

	epoll_event event;
	event.data.fd = listenFD;
	event.events = EPOLLIN;
	epoll_ctl(epollFD, EPOLL_CTL_ADD, listenFD, &event);

	result = listen(listenFD, 1024);
	if (result == -1)
	{
		cout << "listen:" << strerror(errno) << endl;
		return -1;
	}

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
					sockaddr_in clientAddress;
					socklen_t clientAddressLength = sizeof(clientAddress);
					int clientFD = accept(listenFD, (sockaddr*)&clientAddress, &clientAddressLength);
					if (clientFD == -1)
					{
						cout << "accept:" << strerror(errno) << endl;
						return -1;
					}
					int oldOption = fcntl(clientFD, F_GETFL);
					int newOption = oldOption | O_NONBLOCK;
					fcntl(clientFD, F_SETFL, newOption);

					epoll_event event;
					event.data.fd = clientFD;
					event.events = EPOLLIN;
					epoll_ctl(epollFD, EPOLL_CTL_ADD, clientFD, &event);
				}
			}
			else if (ev.events & EPOLLIN)
			{
				char buffer[1024];
				memset(buffer, 0, sizeof(buffer));
				result = recv(fd, buffer, sizeof(buffer), 0);
				cout << buffer << endl;

				epoll_event event;
				event.data.fd = fd;
				event.events = EPOLLOUT;
				epoll_ctl(epollFD, EPOLL_CTL_MOD, fd, &event);
			}
			else if (ev.events & EPOLLOUT)
			{
				string message("server");
				result = send(fd, message.c_str(), message.size(), 0);
			}
		}
	}

	return 0;
}