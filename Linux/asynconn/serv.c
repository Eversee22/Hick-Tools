#include "sockh.h"
#include "comm.h"

Client clients[CLI_MAXNUM];
char buf[MAXLINE];

void doit(int i, int j, FILE* fp){
	int socki, sockj;
	int maxfdp1, n;
	int off=0, iclose, jclose;
	unsigned int mask;
	
	socki = clients[i].sockfd;
	sockj = clients[j].sockfd;

	srand((unsigned int)getpid());
	mask = rand() + 1;
	writen(socki, &mask, sizeof(int));
	writen(sockj, &mask, sizeof(int));

	fd_set rset;
	FD_ZERO(&rset);
	iclose = jclose = 0;
		
	
	for ( ; ; ){
		if (!iclose)
			FD_SET(socki, &rset);
		if (!jclose)
			FD_SET(sockj, &rset);
		maxfdp1 = max(socki, sockj) + 1;
		n = select(maxfdp1, &rset, NULL, NULL, NULL);
		
		if (n < 0){
			fmtlogWrite(fp, "pid%d: select error\n", getpid());
			continue;
		}

		if (FD_ISSET(socki, &rset)){
			n = read(socki, buf, MAXLINE);
			if (n == 0 && !iclose){
				shutdown(socki, SHUT_RD);
				FD_CLR(socki, &rset);
				off += 1;
				iclose = 1;
			} else {
				writen(sockj, buf, n);
			}
		}

		if (FD_ISSET(sockj, &rset)){
			n = read(sockj, buf, MAXLINE);
			if (n == 0 && !jclose){
				shutdown(sockj, SHUT_RD);
				FD_CLR(sockj, &rset);
				off += 1;
				jclose = 1;
			} else {
				writen(socki, buf, n);
			}
		}

		if (off == 2){
			writen(socki, "BYE\n", 4);
			writen(sockj, "BYE\n", 4);
			shutdown(socki, SHUT_WR);
			shutdown(sockj, SHUT_WR);
			break;
		}
	}
}

void sig_chld(int signo){
    pid_t pid;
    int stat;
    while((pid=waitpid(-1,&stat,WNOHANG)) > 0) //wait child
    {
	    printf("child %d terminated\n", pid);
    }
    return;
}    

int main(int argc, char **argv){
	int i, j, maxi, maxfd, listenfd, connfd, sockfd;
	//int nready;
	ssize_t n;
	//fd_set rset, allset;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	char ips[INET_ADDRSTRLEN];
	FILE* fp = NULL;
	int backlog;
	char filename[256] = {"all.log"};
	
	if (argc == 2)
		strcpy(filename, argv[1]);
    fp = fopen(filename, "w");
	if (fp == NULL){
		fp = stdout;
	}

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0){
		fmtlogWrite(fp, "socket error, %s\n", strerror(errno));
		fclose(fp);
		exit(1);
	}
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	
	if (bind(listenfd, (SA*)&servaddr, sizeof(servaddr)) < 0) {
		fmtlogWrite(fp, "bind error, %s\n", strerror(errno));
		fclose(fp);
		exit(1);
	}

	char* ptr;
	if ((ptr = getenv("LISTENQ")) != NULL) // from ENV
		backlog = atoi(ptr);
	else
		backlog = LISTENQ;

	if (listen(listenfd, backlog) < 0){
		fmtlogWrite(fp, "listen error, %s\n", strerror(errno));
		fclose(fp);
		exit(1);
	}
    
    	signal(SIGCHLD, sig_chld);

	maxi = -1;
	for (i=0; i < CLI_MAXNUM; i++){
		bzero(&clients[i], sizeof(clients[i]));
		clients[i].sockfd = -1;
	}

	for ( ; ; ) {
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (SA*)&cliaddr, &clilen);
		printf("accept\n");

		for (i=0; i<CLI_MAXNUM; i++)
			if (clients[i].sockfd < 0){
				clients[i].sockfd = connfd;
				inet_ntop(AF_INET, &cliaddr.sin_addr, clients[i].ip, sizeof(clients[i].ip));
				printf("%s\n", clients[i].ip);
				break;
			}
		if (i == CLI_MAXNUM){
			logWriteln(fp, "too many clients");
			continue;
		}
		
		if (i>maxi)
			maxi = i;
		int pair = 0;
		for(j = 0; j <= maxi && j != i; j++)
			if (clients[j].sockfd > 0 && clients[j].state == 0){
				pair = 1;
				break;
			}
		
		if (!pair){
			printf("not pair\n");
			continue;
		}
		printf("paired\n");
		pid_t pid;
		if ((pid = fork()) == 0) //child
            		{ 
			close(listenfd);
			doit(i, j, fp);
			exit(0);
		}
		close(clients[i].sockfd);
		close(clients[j].sockfd);
		bzero(&clients[i], sizeof(clients[i]));
		bzero(&clients[j], sizeof(clients[j]));	
		clients[i].sockfd = clients[j].sockfd = -1;
	}
}

