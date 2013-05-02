
/*
 * file: webserver.c
 * author: yuanwei
 * version: 0.1
 * date: 
 */

/*
 * define micro
 */
#define PORT_STR_SIZE 6
#define MAX_LINE 100
#define MAX_REQUEST 1000
#define MAX_RW 100
#define MAX_COMMAND 20

#define GET 1
#define PUT 2

/*
 * head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

/*
 * declare function
 */
static int getPort(int *port);
static int getRequest(int cfd, char *request_buf);
static int getCommand(char *request_buf, int *command);
static int doGet(int cfd, char *request_buf);
static int getPath(char *path, char *request_buf);
static int getHomePath(char *path);
static int sendFile(int cfd, char *path);
static int status500(int cfd);
static int status501(int cfd);
static int status404(int cfd, int isLongConnect);



/*
 *
 */
int init(int *sfd)
{
	struct sockaddr_in sin;
	int port;
	
	/* get port number from congfigure file */
	getPort(&port);
	printf("%d\n", port);
	
	bzero(&sin, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);
	*sfd = socket(AF_INET, SOCK_STREAM, 0);
	bind(*sfd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in));
	listen(*sfd, 3);

	return 0;
}

/*
 *
 */
int doRequest(int cfd)
{
	char request_buf[MAX_REQUEST];
	int command;
	int isLongConnect;

	do
	{
		printf("start!\n");
		getRequest(cfd, request_buf);
		if(NULL == strstr(request_buf, "Connection: close"))
		{
			isLongConnect = 1;
		}
		else
		{
			isLongConnect = 0;
		}

		getCommand(request_buf, &command);			
		switch(command)
		{
		case GET:
			doGet(cfd, request_buf);
			break;
		default:
			status501(cfd);
			exit(-1);
		}
		printf("end!\n");
	}while(isLongConnect);

	return 0;
}

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
	readchar = '\0';
	while(':' != readchar)
	{
		read(config_fd, &readchar, 1);
	}
	m = 0;
	while('\n' != readchar)
	{
		read(config_fd, &readchar, 1);
		port_str[m++] = readchar;	
	}
	port_str[m] = '\0';
	*port = atoi(port_str);

	close(config_fd);

	return 0;
}

/*
 *
 */
int getRequest(int cfd, char *request_buf)
{
	int sum;
	int count;
	char buf[MAX_REQUEST];
	int flag;
	
	/* get request. */
	flag = 1;
	sum = 0;
	while(flag)
	{
		//printf("read start!\n");
		read(cfd, buf + sum, 1);
		//printf("read end!\n");
		if('\n' == *(buf + sum))
		{
			//printf("read1\n");
			sum++;
			read(cfd, buf + sum, 1);
			if('\r' == *(buf + sum))
			{
				//printf("read2\n");
				flag = 0;
			}
		}
		sum++;
	}
	read(cfd, buf + sum, 1);
	buf[sum + 1] = '\0';
	if(sum + 1 > MAX_REQUEST)
	{
		printf("array overstep!\n");
		status500(cfd);
		exit(-1);
	}
	strcpy(request_buf, buf);
	printf("receive request from client:\n%s......\n", request_buf);

	return 0;
}

/*
 *
 */
int getCommand(char *request_buf, int *command)
{
	char buf[MAX_LINE];

	memcpy(buf, request_buf, MAX_LINE-1);
	buf[MAX_LINE - 1] = '\0';
	if(strstr(buf, "GET") == buf)
	{
		*command = GET;
	}
	else
	{
	}
	
	return 0;
}

/*
 *
 */
int doGet(int cfd, char *request_buf)
{
	char path[MAX_LINE];
	struct stat statbuf;
	int isLongConnect;
	char buf[30];
	
	if(NULL == strstr(request_buf, "Connection: close"))
	{
		isLongConnect = 1;
	}
	else
	{
		isLongConnect = 0;
	}

	/* get path from request of client */
	getPath(path, request_buf);
			
	/* see the file is exist or not .*/
	if(-1 == access(path, 0))
	{
		perror("access fail");
		status404(cfd, isLongConnect);
		return -1;	
	}
	
	/* process according to type of file */
	stat(path, &statbuf);
	if(!S_ISREG(statbuf.st_mode))
	{
		status404(cfd, isLongConnect);
		return -1;
	}

	if(statbuf.st_mode & S_IXOTH)
	{
		dup2(cfd, STDOUT_FILENO);
		execl(path, path, NULL);
	}

	write(cfd, "HTTP/1.1 200 OK\r\n", strlen("HTTP/1.1 200 OK\r\n"));
	sprintf(buf, "Content-Length: %d\r\n", statbuf.st_size);
	write(cfd, buf, strlen(buf));
	if(isLongConnect)
	{
		write(cfd, "Connection: keep-alive\r\n", strlen("Connection: keep-alive\r\n"));
	}
	write(cfd, "Content-Type: text/html\r\n\r\n", strlen("Content-Type: text/html\r\n\r\n"));
	sendFile(cfd, path);
	
	return 0;
}

/*
 *
 */
int getPath(char *path, char *request_buf)
{
	char buf[MAX_LINE];
	char hpath[MAX_LINE];
	
	getHomePath(hpath);

	memcpy(buf, request_buf, MAX_LINE-1);
	buf[MAX_LINE - 1] = '\0';
	strcpy(path, hpath);

	if(('/' == buf[4]) && (' ' == buf[5]))
	{
		strcat(path, "/index.html");
	}
	else
	{
		strtok(&buf[4], " ");
		strcat(path, &buf[4]);
	}

	return 0;
}

/*
 * function name: getHomePath
 * param[in]:
 * param[out]: home path
 * return: status -1 fail and 0 success
 * decribe: get home path from ./webserver.config
 */
int getHomePath(char *path)
{
	int config_fd;
        char readchar;
	char lpath[MAX_LINE];
        int m;

        config_fd = open("./webserver.config", O_RDONLY);
        readchar = '\0';
        while('\n' != readchar)
        {
                read(config_fd, &readchar, 1);
        }
	while(':' != readchar)
        {
                read(config_fd, &readchar, 1);
        }
        m = 0;
	read(config_fd, &readchar, 1);
        while('\n' != readchar)
        {
                lpath[m++] = readchar;
                read(config_fd, &readchar, 1);
	}
	lpath[m] = '\0';
	
	strcpy(path, lpath);

	return 0;
}


/*
 *
 */
int sendFile(int cfd, char *path)
{
	int fd;
	int count;
	char buf[MAX_RW];

	fd = open(path, O_RDONLY);
	count = read(fd, buf, MAX_RW);
	while(count != 0)
	{
		write(cfd, buf, count);
		count = read(fd, buf, MAX_RW);
	}
	close(fd);

	return 0;
}

/*
 *
 */
int status404(int cfd, int isLongConnect)
{
	char buf[200];

	if(isLongConnect)
	{
		sprintf(buf, 
			"HTTP/1.1 404 Not Exist\r\n"
			"Connection: keep-alive\r\n"
			"Content-Length: %d\r\n"
			"Content-Type: text/html\r\n\r\n"
			"<html>\n"
			"<head><title>error</title></head>\n"
			"<body>\n"
			"<p>This file does not exist!</p>\n"
			"</body>\n"
			"</html>\n", 
			strlen( "<html>\n"
				"<head><title>error</title></head>\n"
				"<body>\n"
				"<p>This file does not exist!</p>\n"
				"</body>\n"
				"</html>\n"
				)
			);
	}
	else
	{
		sprintf(buf, 
			"HTTP/1.1 404 Not Exist\r\n"
			"Content-Type: text/html\r\n\r\n"
			"<html>\n"
			"<head><title>error</title></head>\n"
			"<body>\n"
			"<p>This file does not exist!</p>\n"
			"</body>\n"
			"</html>\n"
			);
	}
	write(cfd, buf, strlen(buf));

	return 0;
}

/*
 *
 */
int status500(int cfd)
{
	dup2(cfd, STDOUT_FILENO);
	printf( "HTTP/1.1 500 Internal Server Error\r\n"
		"Content-Type: text/html\r\n\r\n"
		"<html>\n"
		"<head><title>error</title></head>\n"
		"<body>\n"
		"<p>The server had some problem!</p>\n"
		"</body>\n"
		"</html>\n"
		);

	return 0;
}

/*
 *
 */
int status501(int cfd)
{
	dup2(cfd, STDOUT_FILENO);
	printf( "HTTP/1.1 501 Not Implemented\r\n"
		"Content-Type: text/html\r\n\r\n"
		"<html>\n"
		"<head><title>error</title></head>\n"
		"<body>\n"
		"<p>This operate does not support!</p>\n"
		"</body>\n"
		"</html>\n"
		);

	return 0;
}






