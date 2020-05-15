#ifndef __comm_h
#define __comm_h

//#include <string>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h> // for vsprintf

#define MAXNAME 256
#define MAXPWD 256
#define MAXLOGLINE 256

#define max(a,b)  ((a) > (b) ? (a) : (b))

typedef struct _Client {
	char name[MAXNAME];
	char ip[16];
	int key;
	int sockfd;
	int state;
} Client;

typedef struct _Admin{
	char name[MAXNAME];
	char passwd[MAXPWD];
	char ip[16];
	int key;
	int sockfd;
	int state;
} Admin;

size_t logWrite(FILE* fp, const char* msg){
	size_t n = fwrite(msg, sizeof(char), strlen(msg), fp);
	return n;
}

size_t logWriteln(FILE* fp, const char* msg){
	size_t msglen = strlen(msg)+2;
	char* buf = malloc(msglen*sizeof(char));
	if (buf == NULL)
		return 0;
	bzero(buf, msglen);
	strcpy(buf, msg);
	strcat(buf,"\n");
	size_t n = logWrite(fp, buf);
	free(buf);
	return n;
}

size_t fmtlogWrite(FILE* fp, const char* fmt, ...) {
	char* buf;
	buf = malloc(MAXLOGLINE*sizeof(char));
	if (buf == NULL )
		return 0;
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buf,fmt,ap);
	va_end(ap);
	size_t n = logWrite(fp, buf);
	free(buf);
	return n;
}

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

size_t Writen(FILE* fp, int fd, const void* vptr, size_t n){
	size_t c;
	if ((c=writen(fd, vptr, n)) < 0){
		logWriteln(fp, "write error");
	}
	return n;
}		


#endif //__comm_h


