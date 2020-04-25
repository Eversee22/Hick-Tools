/* group message */
#include "sockh.h"
#include <string>
//#include <unordered_map>
#include <vector>

using namespace std;

struct Client {
    string ip;
    string name;
    int sockfd;
};

int main(int argc, char **argv){
	int					i, maxi, maxfd, listenfd, connfd, sockfd;
	int					nready;
    Client          	client[FD_SETSIZE];
	ssize_t				n;
	fd_set				rset, allset;
	char				buf[MAXLINE];
	socklen_t			clilen;
	struct sockaddr_in	cliaddr, servaddr;
	vector<string> climsg;
	string servmsg;
	char ips[INET_ADDRSTRLEN];

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(SERV_PORT);

	Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));

	Listen(listenfd, LISTENQ);

	maxfd = listenfd;			/* initialize */
	maxi = -1;					/* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i].sockfd = -1;			/* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	for ( ; ; ) {
		rset = allset;		/* structure assignment */
		nready = Select(maxfd+1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(listenfd, &rset)) {	/* new client connection */
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);
#ifdef	NOTDEF
			printf("new client: %s, port %d\n",
					Inet_ntop(AF_INET, &cliaddr.sin_addr, ips, sizeof(ips)),
					ntohs(cliaddr.sin_port));
#endif

			for (i = 0; i < FD_SETSIZE; i++)
				if (client[i].sockfd < 0) {
					client[i].sockfd = connfd;	/* save descriptor */
					client[i].ip = Inet_ntop(AF_INET, &cliaddr.sin_addr, ips, sizeof(ips));
					break;
				}
			if (i == FD_SETSIZE){
				err_ret("too many clients");
			}
			else{

				FD_SET(connfd, &allset);	/* add new descriptor to set */
				if (connfd > maxfd)
					maxfd = connfd;			/* for select */
				if (i > maxi)
					maxi = i;				/* max index in client[] array */

				servmsg = "Name needed\n";
				Writen(connfd, servmsg.c_str(),servmsg.length()); 

				if ((n = Read(connfd, buf, MAXLINE)) == 0){
					Close(connfd);
					FD_CLR(connfd, &allset);
					client[i].sockfd = -1;
					client[i].ip = "";
				} else {
					if (buf[n-1] == '\n') //remove newline
					{
						buf[n-1] = '\0';
						if (n>1 && buf[n-2] == '\r')
						       buf[n-2] = '\0';	
					}
					else
						buf[n] = '\0';
					client[i].name = buf;
				}
			}
			
			if (--nready <= 0)
				continue;				/* no more readable descriptors */
		}

		for (i = 0; i <= maxi; i++) {	/* check all clients for data */
			if ( (sockfd = client[i].sockfd) < 0)
				continue;
			if (FD_ISSET(sockfd, &rset)) {
				if ( (n = Read(sockfd, buf, MAXLINE)) == 0) {
						/*4connection closed by client */
					Close(sockfd);
					FD_CLR(sockfd, &allset);
					client[i].sockfd = -1;
					client[i].name="";
					client[i].ip="";
				} else{
					buf[n] = '\0';
					//servmsg = buf;
				    servmsg = client[i].name+"@"+client[i].ip+": "+buf;
					climsg.push_back(servmsg);
				}
				
				if (--nready <= 0){
					for (int j=0; j<=maxi; j++){
						if ((sockfd = client[j].sockfd) >= 0){
							for (int k=0; k<climsg.size(); k++){
								servmsg = climsg[k];
								Writen(sockfd, servmsg.c_str(), servmsg.length());
							}
						}
					}
					climsg.clear();
					break;				/* no more readable descriptors */
				}
			}
		}
	}
}
