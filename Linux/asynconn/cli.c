#include "sockh.h"
#include "comm.h"

unsigned char a,b;

struct in_addr getipbyname(const char* name){
    struct hostent* hp;
    struct in_addr** pptr;
    struct in_addr inetaddr;
    
    if ((hp = gethostbyname(name)) == NULL) {
        if (inet_aton(name, &inetaddr) != 0) {
            return inetaddr;
        } else {
            printf("hostname error for %s\n", name);
            exit(1);
        }
    } else {
        pptr = (struct in_addr**)hp->h_addr_list;
        return *pptr[0];
    }
}

void encd(char* buf, int n){
	int i;
	for (i = 0; i<n; i++){
		if (i%2 == 0)
			buf[i] ^= a;
		else
			buf[i] ^= b;
	}
}

void printb(unsigned char* buf, int n){
	int i;
	for(i=0; i<n; ++i)
		printf("%02x", buf[i]);
	printf("\n");
}

void doit(FILE* fp, int sockfd){
    int maxfdp1, stdineof;
    fd_set rset;
    char buf[MAXLINE];
    int n;
    
    stdineof = 0;
    FD_ZERO(&rset);
    for ( ; ; ){
        if (stdineof == 0)
            FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfdp1 = max(fileno(fp), sockfd) + 1;
        select(maxfdp1, &rset, NULL, NULL, NULL);
        
        if (FD_ISSET(sockfd, &rset)) {
            if ((n=read(sockfd, buf, MAXLINE)) == 0){
                if (stdineof == 1)
                    return;
                else {
                    logWriteln(fp, "server terminated");
                    exit(1);
                }
            }
	    printf("recv:\n");
	    printb(buf, n);
	    //strcat(buf,"\n");
            //write(fileno(stdout), buf, n+1);
	    encd(buf, n);
	    //write(fileno(stdout), buf, n);
	    printf("encd:\n");
	    printb(buf,n);
        }
        
        if (FD_ISSET(fileno(fp), &rset)) {
            if ((n=read(fileno(fp), buf, MAXLINE)) == 0){
                stdineof = 1;
                shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fp), &rset);
                continue;
            }
	    printf("input:\n");
	    printb(buf,n);
	    encd(buf, n);
	    //strcat(buf,"\n");
	    //write(fileno(stdout), buf, n+1);
	    printf("encd:\n");
	    printb(buf,n);
            writen(sockfd, buf, n);
        }
    }          
}

int main(int argc, char** argv){
    int sockfd;
    struct in_addr ipaddr;
    struct sockaddr_in servaddr;
    int n;
    char inetaddr[16] = "127.0.0.1";
    char buf[MAXLINE];
    
    if (argc == 2) {
        strncpy(inetaddr, argv[1], 16);
    }
    
    ipaddr = getipbyname(inetaddr);
    printf("ip: %s\n", inet_ntoa(ipaddr));
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    memcpy(&servaddr.sin_addr, &ipaddr, sizeof(struct in_addr));
    connect(sockfd, (SA*)&servaddr, sizeof(servaddr));
    printf("connected\n");
    n=read(sockfd, buf, sizeof(int));
    if (n==0){
	    printf("serv terminated\n");
	    exit(1);
    }
    a = buf[0] | 0x80;
    b = buf[1] | 0x80;
    printf("get mask: %02x, %02x\n", a, b);
    doit(stdin, sockfd);
    exit(0);
}
    
    
    
    
    
