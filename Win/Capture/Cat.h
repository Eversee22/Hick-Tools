#ifndef HICK_TOOLS_CAT_H
#define HICK_TOOLS_CAT_H

#include"win_common.h"
#include<iostream>
#include<string>
using namespace std;

typedef struct _iphdr {
	unsigned char h_verlen;         //4 bits header length and 4 bits IP version
	unsigned char tos;              //8 bits TOS(Type Of Service)
	unsigned short total_len;       //16 bits total length (byte)
	unsigned short ident;           //16 bits identification
	unsigned short frag_and_flags;  //3 bits flag and 13 bits fragment offset
	unsigned char ttl;              //8 bits TTL
	unsigned char proto;            //8 bits protocol(TCP, UDP or other)
	unsigned short checksum;        //16 bits ip header checksum
	unsigned int sourceIP;          //32 bits source ip address
	unsigned int destIP;            //32 bits destination address
}IPHeader;

typedef struct _udphdr{
	unsigned short uh_sport;    //16 bits source port
	unsigned short uh_dport;    //16 bits destination port
	unsigned short uh_len;      //16 bits udp packet length
	unsigned short uh_sum;      //16 bits checksum
}UDPHeader;

typedef struct _tcphdr {
	unsigned short th_sport;    //16 bits source port
	unsigned short th_dport;    //16 bits destination port
	unsigned int th_seq;       //32 bits Seq. number
	unsigned int th_ack;       //32 bits Ack. number
	unsigned char th_lenres;    //4 bits header length/6 bits reservation
	unsigned char th_flag;      //6 bits flag
	unsigned short th_win;      //16 bits window size
	unsigned short th_sum;      //16 bits checksum
	unsigned short th_urp;      //16 bits urgent data offset
}TCPHeader;

typedef struct _icmphdr {
	unsigned char icmp_type;
	unsigned char icmp_code;
	unsigned short icmp_cksum;
	unsigned short icmp_id;  //unstandard icmp header
	unsigned short icmp_seq;
	//unsigned  icmp_timestamp;
}ICMPHeader;

typedef int(CALLBACK* CaptureDef)(string &strMsg);

class HickCat {
public:
	HickCat();
	~HickCat();
	 void startCat();
	 void stopCat();
	 void setTCP(bool value);
	 void setUDP(bool value);
	 void setICMP(bool value);
	 void setAll(bool value);
	 
private:
	IPHeader * pip;
	TCPHeader *ptcp;
	UDPHeader *pudp;
	ICMPHeader *picmp;

	int sockfd;
	char buf[10240];
	int n, iplen;
	
	//for filters
	bool m_bCapTCP;
	bool m_bCapUDP;
	bool m_bCapICMP;
	//string m_strSrcIP;
	//string m_strDstIP;
	

	void analyseIP();
	void analyseTCP();
	void analyseUDP();
	void analyseICMP();
	void catching();
	void printData(char * pdata);
};


#endif //HICK_TOOLS_CAT_H