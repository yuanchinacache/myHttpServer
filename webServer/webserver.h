
#ifndef WEBSERVER_H
#define WEBSERVER_H

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
#include <sys/epoll.h>

/*
 * define micro
 */
#define PORT_STR_SIZE 6
#define MAX_LINE 100
#define MAX_REQUEST 2000
#define MAX_RW 100
#define MAX_COMMAND 20

#define GET 1
#define PUT 2

#define ON 1
#define OFF 0

#define HOME_PATH "./webserver.config"

/*
 * declare function
 */
int init(int *sfd);
int getRequest(int cfd, char *request_buf);
int getCommand(char *request_buf, int *command);
int doGet(int cfd, char *request_buf);
int status500(int cfd);
int status501(int cfd);
int status404(int cfd, int isLongConnect);

#endif
