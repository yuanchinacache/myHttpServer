
/*
 * head files
 */
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include "webserver.h"

/*
 *
 */
typedef struct tharg
{
	int cfd;
}THARG;

/*
 *
 */
void *thfn (void *arg)
{
	int sfd;
	int cfd;

	cfd = ((THARG *)arg)->cfd;
	
//	close(sfd);
	doRequest(cfd);
	close(cfd);

	return NULL;
}

/*
 * main function
 */
int main(int argc, char *argv[])
{
	struct sockaddr_in cin;
	socklen_t csocklen = sizeof(cin);
	int sfd, cfd;

//	pid_t pid;

	pthread_t tid;
	THARG arg;
	
	init(&sfd);

	while(1)
	{
		cfd = accept(sfd, (struct sockaddr *)&cin, &csocklen);
		printf("accept!\n");

		/*It may result in zomble process.*/
//		pid = fork();
		arg.cfd = cfd;
		pthread_create(&tid, NULL, thfn, (void *)&arg);

//		if(0 == pid)
//		{
//			close(sfd);
//			doRequest(cfd);
//			close(cfd);
//			exit(0);
//		}
//		else
//		{
//			close(cfd);
//		}
	}

	return 0;
}
