#define _CRT_SECURE_NO_WARNINGS 
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include"Cat.h"
#include<stdlib.h>
//#include<mstcpip.h> /*for SIO_RCVALL*/

#define MAXDATA 100
#define M_SIO_RCVALL (IOC_IN|IOC_VENDOR|1)



HickCat::HickCat() {
	setAll(false);
}

HickCat::~HickCat()
{
	stopCat();
}

void HickCat::startCat(){
	WSADATA wsaData;
	DWORD dwValue;
	char LocalName[256];
	struct hostent* pHost;
	//int flag = 1;
	unsigned int optval = 1;
	SOCKADDR_IN addr_in;


	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		printf("WSAStartup error ! %d",GetLastError());
		exit(1);
	}

	if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_IP)) == SOCKET_ERROR)
	{
		printf("socket error !");
		exit(1);
	}

	/*if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, (char*)&flag, sizeof(flag))!=0) {
		printf("setsockopt error ! %d", WSAGetLastError());
	}*/

	if (gethostname(LocalName, sizeof(LocalName) - 1) == SOCKET_ERROR) {
		printf("get hostname error ! %d", WSAGetLastError());
		exit(1);
	}

	if ((pHost = gethostbyname(LocalName)) == NULL) {
		printf("gethostbyname error ! %d", WSAGetLastError());
		exit(1);
	}

	addr_in.sin_addr = *((in_addr*)pHost->h_addr_list[0]);
	addr_in.sin_family = AF_INET;
	addr_in.sin_port = htons(48965);
	
	if (bind(sockfd, (struct sockaddr*)&addr_in, sizeof(addr_in)) != 0) {
		printf("bind error ! %d", WSAGetLastError());
		exit(1);
	}
	/*WSAIoctl(sockfd, M_SIO_RCVALL, &optval, sizeof(optval), NULL, 0, &dwValue, NULL, NULL)*/
	if (WSAIoctl(sockfd, M_SIO_RCVALL, &optval, sizeof(optval), NULL, 0, &dwValue, NULL, NULL) !=0) {
		printf("ioctlsocket error ! %d", WSAGetLastError());
		exit(1);
	}
	catching();

}

void HickCat::stopCat()
{
	closesocket(sockfd);
	WSACleanup();
}



void HickCat::catching() {
	unsigned long pcount = 0;

	while (true) {

		n = recv(sockfd, buf, sizeof(buf), 0);

		if (n<0) {

			printf("recv error !\n");

			break;

		}
		else if (n == 0) {

			continue;

		}
		printf("packet(%d)\n", ++pcount);

		pip = (IPHeader*)buf;
		iplen = (pip->h_verlen & 0x0f) * 4;

		if (pip->proto == IPPROTO_TCP&&m_bCapTCP) {

			ptcp = (TCPHeader*)(buf + iplen);
			analyseIP();
			analyseTCP();

		}
		else if (pip->proto == IPPROTO_UDP&&m_bCapUDP) {

			pudp = (UDPHeader*)(buf + iplen);
			analyseIP();
			analyseUDP();

		}
		else if (pip->proto == IPPROTO_ICMP&&m_bCapICMP) {

			picmp = (ICMPHeader*)(buf + iplen);
			analyseIP();
			analyseICMP();

		}
		else{

			//printf("\nother protocol !\n\n");

		}

		//printf("\n\n");

	}

	return;

}



void HickCat::setTCP(bool value)
{
	m_bCapTCP = value;
}

void HickCat::setUDP(bool value)
{
	m_bCapUDP = value;
}

void HickCat::setICMP(bool value)
{
	m_bCapICMP = value;
}

void HickCat::setAll(bool value) {
	m_bCapTCP = value;
	m_bCapUDP = value;
	m_bCapICMP = value;
}

void HickCat::analyseIP() {
	printf("IP ----- \n"
		"total length: %d bytes\n"
		"recv length: %d bytes\n"
		"header length: %d bytes\n",
		pip->total_len, n, iplen);

	unsigned char * p = (unsigned char*)&pip->sourceIP;
	printf("Source IP :%u.%u.%u.%u\n", p[0], p[1], p[2], p[3]);
	p = (unsigned char *)&pip->destIP;
	printf("Destination IP: %u.%u.%u.%u\n", p[0], p[1], p[2], p[3]);



}



void HickCat::analyseTCP() {

	int tcplen = (ptcp->th_lenres >> 4) * 4;

	printf("TCP ----- (header: %d bytes)\n", tcplen);

	printf("Source port: %u\n", ntohs(ptcp->th_sport));

	printf("Destination port: %u\n", ntohs(ptcp->th_dport));

	printData((char*)ptcp + tcplen);



}



void HickCat::analyseUDP() {

	printf("UDP -----\n");

	printf("Source port: %u\n", ntohs(pudp->uh_sport));

	printf("Destination port: %u\n", ntohs(pudp->uh_dport));

	printData((char*)pudp + sizeof(UDPHeader));

}



void HickCat::analyseICMP() {

	printf("ICMP ------\n");

	printf("type: %u\n", picmp->icmp_type);

	printf("code: %u\n", picmp->icmp_code);

	printData((char*)picmp + sizeof(ICMPHeader));

}



void HickCat::printData(char *pdata) {

	int len = n - (pdata - buf);

	printf("Data(%d bytes): ", len);

	for (int i = 0; i<MAXDATA&&i<len; i++) {

		printf("%c", pdata[i]);

	}

	printf("\n\n");

}