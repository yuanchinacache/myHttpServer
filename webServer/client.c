/*
 * file: client.c
 * author: 
 * version: 0.1
 * date: 
 */

/*
 * define micro
 */
#define PORT_STR_SIZE 6

/*
 * head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>


/*
 * function name: getPort
 * param[in]:
 * param[out]: port number
 * return: status -1 fail and 0 success
 * describe: get port number from configure file ./webserver.config
 */
int getPort(int *port)
{
	int config_fd;
	char port_str[PORT_STR_SIZE];
	char readchar;
	int m;

        config_fd = open("./webserver.config", O_RDONLY);
        if(-1 == config_fd)
        {
		perror("open fail");
		return -1;
        }
	readchar = '\0';
	while(':' != readchar)
	{
		if(-1 == read(config_fd, &readchar, 1))
		{
			perror("read fail");
			return -1;
		}
	}
	m = 0;
	while('\n' != readchar)
	{
		if(-1 == read(config_fd, &readchar, 1))
                {
			perror("read fail");               
                        return -1;
                }
		port_str[m++] = readchar;	
	}
	port_str[m] = '\0';
	*port = atoi(port_str);

	close(config_fd);

	return 0;
}

/*
 * main function
 */

int main(int argc, char *argv[])
{
	int sockfd;
	struct sockaddr_in server_addr;
	int port;
	
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(-1 == sockfd)
	{
		perror("socket fail");
		return -1;
	}
	
	if(-1 == getPort(&port))
	{
		perror("getPort fail");
		return -1;
	}	
	printf("get port number:%d\n", port);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	inet_pton(AF_INET, "192.168.17.161", &server_addr.sin_addr);
	if(-1 == connect(sockfd,(struct sockaddr *)&server_addr, sizeof(server_addr)))
	{
		perror("connect fail");
		return -1;
	}
	printf("connect!\n");

	
	
	close(sockfd);

	return 0;
}
