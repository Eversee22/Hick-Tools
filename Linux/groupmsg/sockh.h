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
#define MAXLINE         4096    /* max text line length */
#define BUFFSIZE        8192    /* buffer size for reads and writes */
#define SERV_PORT       9879                   /* TCP and UDP */
#define SERV_PORT_STR   "9879"                  /* TCP and UDP */

#define SA      struct sockaddr

//func

//#define min(a,b)  ((a) < (b) ? (a) : (b))
//#define max(a,b)  ((a) > (b) ? (a) : (b))

//error
void     err_quit(const char *, ...);
void     err_sys(const char *, ...);
void     err_ret(const char *, ...);
void     err_dump(const char *, ...);
void     err_msg(const char *, ...);

//sock
int      Accept(int, SA *, socklen_t *);
void     Bind(int, const SA *, socklen_t);
void     Connect(int, const SA *, socklen_t);
void     Listen(int, int);
int      Socket(int, int, int);
int      Select(int, fd_set *, fd_set *, fd_set *, struct timeval *);

ssize_t  Read(int, void *, size_t);
void     Close(int);
void     Shutdown(int, int);

void     Write(int, const void *, size_t);
ssize_t  writen(int, const void *, size_t);
void     Writen(int, const void *, size_t);

const char    *Inet_ntop(int, const void *, char *, size_t);
void    Inet_pton(int, const char *, void *);

void str_cli(FILE *, int);
#endif //__sockh_h

