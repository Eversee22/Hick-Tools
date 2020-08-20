/* Use standard echo server; baseline measurements for nonblocking version */
#include	"sockh.h"

int main(int argc, char **argv)
{
	int					sockfd;
	struct sockaddr_in	servaddr;

	char ipaddr[16] = "127.0.0.1";
	if (argc == 2){
		//err_quit("usage: tcpcli <IPaddress>");
		strncpy(ipaddr,argv[1],16);
	}

	sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	Inet_pton(AF_INET, ipaddr, &servaddr.sin_addr);

	Connect(sockfd, (SA *) &servaddr, sizeof(servaddr));

	str_cli(stdin, sockfd);		/* do it all */

	exit(0);
}
