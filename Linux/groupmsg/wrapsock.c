#include "sockh.h"

/* error */
#include        <stdarg.h>              /* ANSI C header file */
#include        <syslog.h>              /* for syslog() */

int             daemon_proc;            /* set nonzero by daemon_init() */

/* Print message and return to caller
 * Caller specifies "errnoflag" and "level" */
static void err_doit(int errnoflag, int level, const char *fmt, va_list ap){
        int             errno_save, n;
        char    buf[MAXLINE + 1];

        errno_save = errno;             /* value caller might want printed */
#ifdef  HAVE_VSNPRINTF
        vsnprintf(buf, MAXLINE, fmt, ap);       /* safe */
#else
        vsprintf(buf, fmt, ap);                                 /* not safe */
#endif
        n = strlen(buf);
        if (errnoflag)
                snprintf(buf + n, MAXLINE - n, ": %s", strerror(errno_save));
        strcat(buf, "\n");

        if (daemon_proc) {
                syslog(level, buf);
        } else {
                fflush(stdout);         /* in case stdout and stderr are the same */
                fputs(buf, stderr);
                fflush(stderr);
        }
        return;
}


/* Fatal error related to system call
 * Print message and terminate */
void err_sys(const char *fmt, ...)
{
        va_list         ap;

        va_start(ap, fmt);
        err_doit(1, LOG_ERR, fmt, ap);
        va_end(ap);
        exit(1);
}

/* Fatal error unrelated to system call
 * Print message and terminate */
void err_quit(const char *fmt, ...)
{
        va_list         ap;

        va_start(ap, fmt);
        err_doit(0, LOG_ERR, fmt, ap);
        va_end(ap);
        exit(1);
}

/* Nonfatal error related to system call
 * Print message and return */

void err_ret(const char *fmt, ...)
{
        va_list         ap;

        va_start(ap, fmt);
        err_doit(1, LOG_INFO, fmt, ap);
        va_end(ap);
        return;
}

/* Fatal error related to system call
 * Print message, dump core, and terminate */

void err_dump(const char *fmt, ...)
{
        va_list         ap;

        va_start(ap, fmt);
        err_doit(1, LOG_ERR, fmt, ap);
        va_end(ap);
        abort();                /* dump core and terminate */
        exit(1);                /* shouldn't get here */
}

/* Nonfatal error unrelated to system call
 * Print message and return */

void err_msg(const char *fmt, ...)
{
        va_list         ap;

        va_start(ap, fmt);
        err_doit(0, LOG_INFO, fmt, ap);
        va_end(ap);
        return;
}

/* end err */


/* wrapsock */
int Socket(int family, int type, int protocol){
        int     n;
        if ( (n = socket(family, type, protocol)) < 0)
                err_sys("socket error");
        return(n);
}

void Bind(int fd, const struct sockaddr *sa, socklen_t salen){
        if (bind(fd, sa, salen) < 0)
                err_sys("bind error");
}

void Listen(int fd, int backlog){
        char    *ptr;

                /*4can override 2nd argument with environment variable */
        if ( (ptr = getenv("LISTENQ")) != NULL)
                backlog = atoi(ptr);

        if (listen(fd, backlog) < 0)
                err_sys("listen error");
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr){
        int             n;

        while ( (n = accept(fd, sa, salenptr)) < 0) {
#ifdef  EPROTO
                if (errno == EPROTO || errno == ECONNABORTED)
#else
                if (errno == ECONNABORTED)
#endif
			continue;
        	err_sys("accept error");
        }
        return(n);
}

void Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
        if (connect(fd, sa, salen) < 0)
                err_sys("connect error");
}

int Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, 
		struct timeval *timeout)
{
        int             n;

        if ( (n = select(nfds, readfds, writefds, exceptfds, timeout)) < 0)
                err_sys("select error");
        return(n);              /* can return 0 on timeout */
}

void Shutdown(int fd, int how){
        if (shutdown(fd, how) < 0)
                err_sys("shutdown error");
}

/* end wrapsock */


/* wrapunix */
ssize_t Read(int fd, void *ptr, size_t nbytes)
{
        ssize_t         n;

        if ( (n = read(fd, ptr, nbytes)) == -1)
                err_sys("read error");
        return(n);
}

void Write(int fd, const void *ptr, size_t nbytes)
{
        if (write(fd, ptr, nbytes) != nbytes)
                err_sys("write error");
}

void Close(int fd){
        if (close(fd) == -1)
                err_sys("close error");
}


/* end wrapunix */


/* writen, Write "n" bytes to a descriptor. */
ssize_t writen(int fd, const void *vptr, size_t n){
        size_t          nleft;
        ssize_t         nwritten;
        const char      *ptr;

        ptr = (const char*)vptr;
        nleft = n;
        while (nleft > 0) {
                if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
                        if (nwritten < 0 && errno == EINTR)
                                nwritten = 0;           /* and call write() again */
                        else
                                return(-1);                     /* error */
                }

                nleft -= nwritten;
                ptr   += nwritten;
        }
        return(n);
}

void Writen(int fd, const void *ptr, size_t nbytes)
{
        if (writen(fd, ptr, nbytes) != nbytes)
                err_sys("writen error");
}
/* end writen */


/* wraplib */
const char *Inet_ntop(int family, const void *addrptr, char *strptr, size_t len)
{
        const char      *ptr;

        if (strptr == NULL)             /* check for old code */
                err_quit("NULL 3rd argument to inet_ntop");
        if ( (ptr = inet_ntop(family, addrptr, strptr, len)) == NULL)
                err_sys("inet_ntop error");             /* sets errno */
        return(ptr);
}

void Inet_pton(int family, const char *strptr, void *addrptr)
{
        int             n;

        if ( (n = inet_pton(family, strptr, addrptr)) < 0)
                err_sys("inet_pton error for %s", strptr);      /* errno set */
        else if (n == 0)
                err_quit("inet_pton error for %s", strptr);     /* errno not set */

        /* nothing to return */
}
/* end wraplib */

