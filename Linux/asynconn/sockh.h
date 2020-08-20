#ifndef __sockh_h
#define __sockh_h

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/times.h>
#include <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>   /* inet(3) functions */
#include <errno.h>
//#include <fcntl.h>               /* for nonblocking */
#include  <netdb.h>
#include  <signal.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
//#include  <sys/stat.h>    /* for S_xxx file mode constants */
//#include  <sys/uio.h>             /* for iovec{} and readv/writev */
#include  <unistd.h>
#include  <sys/wait.h>
//#include  <sys/un.h>              /* for Unix domain sockets */
#include <sys/select.h>


#define LISTENQ         1024    /* 2nd argument to listen() */
/* Miscellaneous constants */
#define MAXLINE         16384    /* max text line length */
#define BUFFSIZE        8192    /* buffer size for reads and writes */
#define SERV_PORT       9881                   /* TCP and UDP */
#define SERV_PORT_STR   "9881"                  /* TCP and UDP */
#define CLI_MAXNUM 4

#define SA      struct sockaddr

//func

//#define min(a,b)  ((a) < (b) ? (a) : (b))
//#define max(a,b)  ((a) > (b) ? (a) : (b))

#endif //__sockh_h

