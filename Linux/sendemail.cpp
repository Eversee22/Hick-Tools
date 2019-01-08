#include<iostream> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <stdio.h> 
#include<cstring> 
#include<unistd.h> 
#include<cstdlib> 
 
using namespace std; 
#define HELO "HELO aa\r\n" 
#define DATA "DATA\r\n" 
#define QUIT "QUIT\r\n" 
#define MAXML 1000

//#define h_addr h_addr_list[0] 
//FILE *fin; 
int sock; 
struct sockaddr_in server; 
struct hostent *hp, *gethostbyname(); 
char buf[BUFSIZ+1]; 
int len; 
char *user="c2VjQGNsb3VkYS5jb20=";
char *pass="YXNkZmdoNDU2";
char *host_id=/*"smtp.gmail.com"*/"172.96.211.103";  //192.168.1.10"; 
char *from_id=/*"caiss.cuny@gmail.com"*/"blacksun"; 
char *to_id=/*"quaere1verum@gmail.com"*/"2071263070@qq.com"; 
char *sub="miss\r\n";
char *from="a nonexistent human\r\n"; 
char *rcp="cute you\r\n";
char wkstr[100]="how are u? I miss you so much ever.\r\n";
char mc[MAXML+3];
 
/*=====Send a string to the socket=====*/ 
 
void send_socket(char *s) 
{ 
        write(sock,s,strlen(s)); 
        write(1,s,strlen(s)); 
        //printf("Client:%s\n",s); 
} 
 
//=====Read a string from the socket=====*/ 
 
void read_socket() 
{ 
        len = read(sock,buf,BUFSIZ); 
        write(1,buf,len); 
        //printf("Server:%s\n",buf); 
}

void writemail()
{
	int count = 0;
	char c, n;
	printf("mail content(end with :q):\n");
	while(count<MAXML){
	  c = getchar();
	  if(c==':'){
		n = getchar();
	    if(n == 'q')
		  break;
	    else{
		  mc[count++] = c;
		  mc[count++] = n;
	    }
	  }else{
	    mc[count++] = c;
	  }
	}
	mc[count++]='\r';
	mc[count++]='\n';
	mc[count++]='\0';
}
	   
/*=====MAIN=====*/
int main(int argc, char* argv[])
{	
		if(argc>=2)
          to_id = argv[1];

		writemail();
	  
        /*=====Create Socket=====*/
        sock = socket(AF_INET, SOCK_STREAM, 0);

        if (sock==-1)
        {
                perror("opening stream socket");
                exit(1);
        }
        else
                cout << "socket created\n";

        /*=====Verify host=====*/
        server.sin_family = AF_INET;
        cout<<"About to call gethost ... \n";
        hp = gethostbyname(host_id);

        cout<<"Return from gethost ... "<< hp<<"\n";
        if (hp==(struct hostent *) 0)
        {
                fprintf(stderr, "%s: unknown host\n", host_id);
                exit(2);
        }
        cout<<"Trying to connect to remote host\n";

        /*=====Connect to port 25 on remote host=====*/
        memcpy((char *) &server.sin_addr, (char *) hp->h_addr, hp->h_length);

        server.sin_port=htons(25); /* SMTP PORT */

        cout<<"Trying to connect to remote host now\n";
        if (connect(sock, (struct sockaddr *) &server, sizeof server)==-1)
        {
                perror("connecting stream socket");
                exit(1);
        }
        else
                cout << "Connected\n";

        /*=====Write some data then read some =====*/
        read_socket(); /* SMTP Server logon string */
        //send_socket("STARTTLS\n");
        //read_socket(); /*Read reply */
        send_socket(HELO); /* introduce ourselves */
        read_socket(); /*Read reply */

        //LOGIN
		send_socket("AUTH LOGIN");
		send_socket("\r\n");
		read_socket();
		send_socket(user);
		send_socket("\r\n");
		read_socket();
		send_socket(pass);
		send_socket("\r\n");
		read_socket(); /*Authentication OK*/
	
        //send_socket("STARTTLS\n");
       
        send_socket("MAIL FROM: ");
        send_socket(from_id);
        send_socket("\r\n");
        read_socket(); /* Sender OK */
        send_socket("VRFY ");
        send_socket(from_id);
        send_socket("\r\n");
        read_socket(); // Sender OK */
        send_socket("RCPT TO: "); /*Mail to*/
        send_socket(to_id);
        send_socket("\r\n");
        read_socket(); // Recipient OK*/
        send_socket(DATA);// body to follow*/
        send_socket("Subject: ");
        send_socket(sub);
        //read_socket(); 
	send_socket("From: ");
        send_socket(from);
        //read_socket(); 
        send_socket("To: ");
        send_socket(rcp);
        //read_socket(); 
        send_socket(mc);
        send_socket(".\r\n");
        read_socket();
        send_socket(QUIT); /* quit */
        read_socket(); // log off */

        //=====Close socket and finish=====*/
        close(sock);
        exit(0);
}
