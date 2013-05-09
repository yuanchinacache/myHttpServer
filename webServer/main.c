
/*
 * head files
 */
#include "webserver.h"

/*
 * main function
 */
int main(int argc, char *argv[])
{
	struct sockaddr_in sin, cin;
	int port;
	int sfd, cfd, epfd;
	struct epoll_event ev, events[10];
	int nfds;
	int m;
	int csocklen;
	char request_buf[10][MAX_REQUEST];
	int command;
	
	init(&sfd);
	
	epfd = epoll_create(50);
	ev.data.fd = sfd;
	ev.events = EPOLLIN | EPOLLET;
	epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev);

	while(1)
	{
		nfds = epoll_wait(epfd, events, 10, 500);

		for(m = 0; m < nfds; m++)
		{
			if(events[m].data.fd == sfd)
			{
				cfd = accept(sfd, (struct sockaddr *)&cin, &csocklen);
				printf("accept!\n");
				ev.data.fd = cfd;
				ev.events = EPOLLIN | EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
			}
			else if(events[m].events & EPOLLIN)
			{
				getRequest(events[m].data.fd, request_buf[m]);
				ev.data.fd = events[m].data.fd;
				ev.events = EPOLLOUT | EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_MOD, events[m].data.fd, &ev);
			}
			else if(events[m].events & EPOLLOUT)
			{
				getCommand(request_buf[m], &command);
				switch(command)
				{
				case GET:
					doGet(events[m].data.fd, request_buf[m]);
					break;
				default:
					status501(events[m].data.fd);
					exit(-1);
				}
				ev.data.fd = events[m].data.fd;
				ev.events = EPOLLIN | EPOLLET;
				epoll_ctl(epfd, EPOLL_CTL_MOD, events[m].data.fd, &ev);
			}
		}
	}

	return 0;
}
