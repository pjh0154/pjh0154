#ifndef tcp_server_HEADER_H
#define tcp_server_HEADER_H

#include "global.h"
#include "ep.h"
#include "fpga_reg.h"
#include "application.h"

#define TCP_CLIENT_DISCONNECT 0
#define TCP_CLIENT_CONNECT 1
#define TCP_CLIENT_CLOSE 2

#define BUFSIZE 4096
#define PORT 9977
#define INVALID_SOCKET ((int)~0)

#define MAX_TCP_CLIENT 1

#define SOCKET_ERROR -1

pthread_t 		ThreadID[MAX_TCP_CLIENT];

struct sockaddr_in 	serveraddr;

struct sockaddr_in 	clientaddr;

NET_CONFIG 		netp;

TCP_QUEUE 		TcpTxQ[TCP_QSIZE];
TCP_QUEUE 		TcpRxQ[TCP_QSIZE];

int 				TcpTxPush;
int 				TcpTxPop;
int 				TcpRxPush;
int 				TcpRxPop;

unsigned int 		TCP_Port;

int 				tcp_status;

char 			client_status[MAX_TCP_CLIENT];

int 				listen_sock;

void net_config_init(void);
void TcpServerInit(void);
void TcpPortInfo(void);
void *AcceptThread(void);
void *RecvThread0(void *client_p);
void *RecvThread1(void *client_p);
void *RecvThread2(void *client_p);
void file_receive(void *client_p,TCP_PACKET *packet ,char *buf);
void TcpPacketPush(int sock,unsigned short id,unsigned short len,unsigned char *ptr);
void *TcpRecvFunction(void);
void TcpPacketPush(int sock,unsigned short id,unsigned short len,unsigned char *ptr);
void TcpAck(int sock,unsigned short sig);
void Tcp_Ack(int sock,unsigned short sig, unsigned short data);
void Tcp_PacketPush(int sock,unsigned short id,unsigned short len,unsigned short *ptr);
void data_ack(unsigned short sig);
void data_send_ext(ADC_EXT *adc_data,int len,int pcnt,int ptcnt);
void ver_ack();
void ld_state_ack();
#endif