#include "../include/tcp_server.h"

extern pthread_mutex_t mutex_lock;
extern int taskSpawn(int priority,int taskfunc);
extern int cprf;
extern int tprf;
extern int pthread_end;
int client_sock[MAX_TCP_CLIENT];

static inline int recvn(int s,char *buf,int len,int flags)
{
	int received;
	char *ptr = buf;
	int left = len;

	while(left>0)
	{
		received = recv(s,ptr,left,flags);
		if(received==SOCKET_ERROR) return SOCKET_ERROR;
		else if(received == 0) return 0;

		left -= received;
		ptr+= received;	
	}
	return(len-left);
}

void TcpAck(int sock,unsigned short sig)
{
	unsigned short info;
	info = 0x0000;
	TcpPacketPush(sock,sig,0,(unsigned char *)&info);
}
/*
void Tcp_Ack(int sock,unsigned short sig, unsigned short data)
{
	unsigned short info;
	info = data;
	Tcp_PacketPush(sock,sig,2,(unsigned short *)&info);
}*/

void Tcp_Ack(int sock,unsigned short sig, unsigned short data)
{
	unsigned short info;
	info = data;
	if(tprf) printf("SEND_DATA : sig_id = %04x\r\n",sig); 
	Tcp_PacketPush(client_sock[0],sig,2,(unsigned short *)&info);
}

void TcpPacketPush(int sock,unsigned short id,unsigned short len,unsigned char *ptr)
{
	TCP_SEND_PACKET packet;
	packet.sig_id=id;
	packet.len=len;
	
	//Tprintf("Send id:0x%04x\r\n",id);
	//Tprintf("Send lentgh:%d\r\n",len);
	if(len>0) {
		memcpy(packet.info,ptr,len);
		send(sock,&packet.sig_id,4 + len,MSG_DONTWAIT|MSG_NOSIGNAL);
	}
	else send(sock,&packet.sig_id,4,MSG_DONTWAIT|MSG_NOSIGNAL);
}

void Tcp_PacketPush(int sock,unsigned short id,unsigned short len,unsigned short *ptr)
{
	TCP_SEND_PACKET packet;
	packet.sig_id=id;
	packet.len=len;
	
	//Tprintf("Send id:0x%04x\r\n",id);
	//Tprintf("Send lentgh:%d\r\n",len);
	if(len>0) {
		memcpy(packet.info,ptr,len);
		send(sock,&packet.sig_id,4 + len,MSG_DONTWAIT|MSG_NOSIGNAL);
	}
	else send(sock,&packet.sig_id,4,MSG_DONTWAIT|MSG_NOSIGNAL);

}

void net_config_init(void)
{

	FILE *fp;
	int i=0,j=0;
	char data,info[4][16],str[80];

	fp = fopen("/run/media/mmcblk0p2/f0/bin/netinfo.cfg","r");

	if(fp) {
		while (!feof(fp)) {
			fread(&data, 1, 1, fp);
			if(data == ' ') {
				info[i][j] = 0;
				i++;
				j = 0;
			}
			else {
				info[i][j] = data;
				j++;
			}
		}
		j--;
		info[i][j] = 0;
		strcpy(netp.myip, info[0]);
		strcpy(netp.serverip, info[1]);
		strcpy(netp.gateway, info[2]);
		strcpy(netp.netmask, info[3]);
   		fclose(fp);
	}
	else {
		LOGE("Network Config file not exist...\n");
		
		strcpy(netp.myip, "192.167.0.10");
		strcpy(netp.serverip, "192.167.0.100");
		strcpy(netp.gateway, "192.167.0.1");
		strcpy(netp.netmask, "255.255.255.0");

		fp = fopen("/run/media/mmcblk0p2/f0/bin/netinfo.cfg","wb");
		
		sprintf(str,"%s %s %s %s",netp.myip,netp.serverip,netp.gateway,netp.netmask);
		
		fwrite(str, strlen(str), 1, fp);
   		fclose(fp);
	}
	
	
	system("/sbin/ifconfig eth0 down");
	sprintf(str, "/sbin/ifconfig eth0 %s", netp.myip);
	system(str);
	system("ifconfig > mac");
	fp = fopen("mac", "r");
	fread(str,60,1,fp);
	for(i=38,j=0;i<56;i++,j++)
		netp.mac[j]=str[i];
	fclose(fp);
	printf("mac : %s\n", netp.mac);
	system("rm mac");

	printf("myip = %s\n",netp.myip);
	printf("serverip = %s\n",netp.serverip);
	printf("gateway = %s\n",netp.gateway);
	printf("netmask = %s\n",netp.netmask);
	printf("Mac addr = %s\n",netp.mac);
}

void TcpServerInit(void)
{
	int retval;
	
	TcpTxPush = 0;
	TcpTxPop = 0;
	TcpRxPush = 0;
	TcpRxPop = 0;
	
	tcp_status = 0;
	
	memset(client_status,0,sizeof(client_status));
	
	TcpPortInfo();
	
	listen_sock = socket(PF_INET,SOCK_STREAM,0);
	
	int optval = 1; 

     setsockopt(listen_sock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof (optval)); 
	
	if(listen_sock==INVALID_SOCKET) printf("SOCKET_ERROR::socket()\n");

	memset(&serveraddr,0,sizeof(serveraddr));
	serveraddr.sin_family=PF_INET;
	serveraddr.sin_port=htons(TCP_Port);
	serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);

	retval =bind(listen_sock,(struct sockaddr*)&serveraddr,sizeof(serveraddr));
	if(retval==SOCKET_ERROR) printf("SOCKET_ERROR::bind()\n");
	
	retval = listen(listen_sock,SOMAXCONN);
	if(retval==SOCKET_ERROR) printf("SOCKET_ERROR::listen()\n");
	
	retval = fcntl(listen_sock, F_GETFL, 0);
	fcntl(listen_sock, F_SETFL, retval | O_NONBLOCK);

	taskSpawn(80,(int)AcceptThread);

	//taskSpawn(81,(int)TcpRecvFunction);
	
	printf("TCP/IP server init ok\n");
	
}

void TcpPortInfo(void)
{
//	FILE *fp;
//	char str[32];
	
	TCP_Port = PORT;
/*	
	memset(str,0,sizeof(str));
	if( 0 != access( "/hdd/config/tcpport.cfg", F_OK))
	{
	      fp = fopen("/hdd/config/tcpport.cfg","wb");  
	      sprintf(str,"%d",TCP_Port);
	      fwrite(str, strlen(str), 1, fp);
	      sync();
	      fclose(fp);
	}
	else
	{
	      fp = fopen("/hdd/config/tcpport.cfg","r");  
	      fgets(str,sizeof(str), fp);
	      sync();
	      fclose(fp);
	      TCP_Port=atoi(str);
	}
*/
	printf("TCP_Port=%d\n",TCP_Port);
}

void *AcceptThread(void)
{
	int i;
	unsigned int addrlen = sizeof(clientaddr);
	int optval = 1; 	
	
	for(;;) 
	{
	     pthread_mutex_lock(&mutex_lock);
		for(i=0;i<MAX_TCP_CLIENT;i++)
		{
			if((client_status[i]==TCP_CLIENT_DISCONNECT)||(client_status[i]==TCP_CLIENT_CLOSE))
			{
				client_sock[i] = accept(listen_sock,(struct sockaddr*)&clientaddr,&addrlen);
			     setsockopt(listen_sock, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof (optval)); 
				if(client_status[i]==TCP_CLIENT_CLOSE) pthread_join(ThreadID[i],NULL);
				if(client_sock[i] == -1) continue;
				else
				{
				      switch(i)
				      {
							case 0: pthread_create(&ThreadID[i], NULL, RecvThread0, (void *)&client_sock[i]); break;
							case 1: pthread_create(&ThreadID[i], NULL, RecvThread1, (void *)&client_sock[i]); break;
							case 2: pthread_create(&ThreadID[i], NULL, RecvThread2, (void *)&client_sock[i]); break;
							default:break;
				      }
				      client_status[i]=TCP_CLIENT_CONNECT;
				}
			}
		}
		pthread_mutex_unlock(&mutex_lock);
		usleep(1000);
	}
	close(listen_sock);
}

void *RecvThread0(void *client_p)
{  
	TCP_PACKET packet;
	char exit=1;
	struct pollfd     poll_events_recv;      // 체크할 event 정보를 갖는 struct
   	int    poll_state_recv;
	char buff[BUFSIZE+5];
	int waitcnt;
	int ret=0;
	printf( "TCP0 CONNECT!!!!\n");
	 // poll 사용을 위한 준비
	poll_events_recv.fd        = (*(int *)client_p);
	//감시 대상인 디스크립터를 지정
	poll_events_recv.events    = POLLIN | POLLERR;          // 수신된 자료가 있는지, 에러가 있는지
	poll_events_recv.revents   = 0;	//revents를 0 으로 청소
	while(exit)
	{
		poll_state_recv = poll(                               // poll()을 호출하여 event 발생 여부 확인     
		(struct pollfd*)&poll_events_recv, // event 등록 변수
			1,  // 체크할 pollfd 개수
			1000   // time out 시간
		);
		if ( 0 < poll_state_recv)                             // 발생한 event 가 있음
		{ 
			//clock_gettime(CLOCK_MONOTONIC, &func_time);
			if ( poll_events_recv.revents & POLLIN)            // event 가 자료 수신?
			{
				ret=recvn(*(int *)client_p,(char *)&packet,4,0);
				if(ret==0)
				{
				      printf( "TCP0 DISCONNECT!!!!\n");
				      exit=0;
				}
				else if(ret<0)
				{
					printf( "TCP0 ERROR : %d!!!!\n",ret);
				}
				else{
					/*waitcnt =0;
					while(waitcnt++< 100) {
						if(tcp_status == IDLE) break;
						usleep(1);
					}
					if(waitcnt == 100) printf("TCP0 wait over time\r\n");*/

					tcp_status = BUSY;

					//pthread_mutex_lock(&mutex_lock);
					if(tprf) printf("TCP0 : sig_id = %04x, len = %d\r\n",packet.sig_id,packet.len); 

					if(packet.sig_id == REQ_FILE_RCV) {
						file_receive(client_p,&packet,buff);
					}
					else {	
						if(packet.len>0) {
							memset(buff,0,BUFSIZE);
							recvn(*(int *)client_p,buff,packet.len,0);
							memcpy(&TcpRxQ[0].info[0], buff, packet.len);
						}
						#if 0
						TcpRxQ[TcpRxPush].port_id = client_p;
						TcpRxQ[TcpRxPush].sig_id = packet.sig_id;
//					    	if(packet.sig_id == PatternId) t = myclock();
						TcpRxQ[TcpRxPush].len = packet.len;
						/*TcpRxPush++;
						if(TcpRxPush == TCP_QSIZE) TcpRxPush = 0;
						*/
						
						if(TcpRxPush == (TCP_QSIZE-1)) TcpRxPush = 0;
						else TcpRxPush++;
						#else 
						//if(TcpRxPush != TcpRxPop) 
						{
							switch(packet.sig_id) 
							{
								case LD_ON_OFF : 
									if(packet.len == 2)
									{
										ld_1d_data.ld_on_off_state = TcpRxQ[0].info[0]; 
										//ld_state = TcpRxQ[0].info[0];
										if(cprf) printf("LD_CTL_DATA = %x\r\n", ld_state);
										Tcp_Ack((int )(&client_p),LD_ON_OFF_ACK, ACK_OK);																																			
									}
									break;

								case ADC_READ_COUNT_TRIGGER_COUNT :
									if(packet.len == 6)
									{
										if(TcpRxQ[0].info[2]	==	TcpRxQ[0].info[0]+TcpRxQ[0].info[1])
										{
											//count_data.read = TcpRxQ[0].info[0];
											count_data.trigger = TcpRxQ[0].info[1];
											//LTC1420_0->sensing_count = count_data.read;
											//LTC1420_1->sensing_count = count_data.read;
											//LTC1420_2->sensing_count = count_data.read;
											ad7656->total_count = TcpRxQ[0].info[1];														 
											real_count_0 = 0;
											real_count_1 = 0;	
											real_count_2 = 0;	
											real_count_3 = 0;	
											real_count_4 = 0;																				
											usleep(10);

											Tcp_Ack((int )(&client_p),ADC_READ_COUNT_TRIGGER_COUNT_ACK, ACK_OK);
										}
										else
										{
											Tcp_Ack((int )(&client_p),ADC_READ_COUNT_TRIGGER_COUNT_ACK, ACK_NG);
										}	
									}
									else	Tcp_Ack((int )(&client_p),ADC_READ_COUNT_TRIGGER_COUNT_ACK, ACK_NG);					 
									break;

									case ADC_SENSING_START : 
										if(packet.len == 2)
										{
											if(TcpRxQ[0].info[0] == 0x0000)
											{	
												formula_calculation_flag = 0;													
												adc_trigger_count_0 = 0;
												adc_trigger_count_1 = 0;
												adc_trigger_count_2 = 0;
												adc_trigger_count_3 = 0;
												adc_trigger_count_4 = 0;												
												data_ack(ACK_OK);														
												read_pro_flag = 0;											
											}
											else if(TcpRxQ[0].info[0] == 0x0001)
											{														
												formula_calculation_flag = 0;																										
												adc_trigger_count_0 = 0;
												adc_trigger_count_1 = 0;
												adc_trigger_count_2 = 0;
												adc_trigger_count_3 = 0;
												adc_trigger_count_4 = 0;												
												data_ack(ACK_OK);
												pro_flag = 0;																																							
											}
											else if(TcpRxQ[0].info[0] == 0x0002)
											{	
												formula_calculation_flag = 1;
												adc_trigger_count_0 = 0;
												adc_trigger_count_1 = 0;
												adc_trigger_count_2 = 0;
												adc_trigger_count_3 = 0;
												adc_trigger_count_4 = 0;												
												data_ack(ACK_OK);														
												read_pro_flag = 0;
											}
											else if(TcpRxQ[0].info[0] == 0x0003)
											{														
												formula_calculation_flag = 1;												
												adc_trigger_count_0 = 0;
												adc_trigger_count_1 = 0;
												adc_trigger_count_2 = 0;
												adc_trigger_count_3 = 0;
												adc_trigger_count_4 = 0;												
												data_ack(ACK_OK);
												pro_flag = 0;
																												
											}																						
											//data_ack(ACK_OK);
										}										
									break;
									case  VERSION_REQUEST :	
										ver_ack();
									break;										
									case  REBOOT_REQUEST :
										printf("*******************************************REBOOT************************************************************\r\n");
										system("reboot");
									break;
									case  FORMULA_CALCULATION_INIT :
									break;  
									case  LD_CURRENT_CTRL :
										if(packet.len == 2)
										{
											if((TcpRxQ[0].info[0] & 0x00ff) == 0x80)	
											{
												ld_1d_data.ld_id = 0x00;
												ld_1d_data.ld_value = (TcpRxQ[0].info[0] >>8) & 0x00ff;	
												if(cprf) printf("LD_NAME = %x / LD_VALUE = %x\r\n", ld_1d_data.ld_id,ld_1d_data.ld_value);													
											}
											else	if((TcpRxQ[0].info[0] & 0x00ff) == 0x00)	
											{
												ld_2d_data.ld_id = 0x80;
												ld_2d_data.ld_value = (TcpRxQ[0].info[0] >>8) & 0x00ff;	
												if(cprf) printf("LD_NAME = %x / LD_VALUE = %x\r\n", ld_2d_data.ld_id,ld_2d_data.ld_value);																										
											}																			
										}									
									break;
									case  LD_CURRENT_VALUE_SAVE :
									break;
									case  LD_STATE_REQUEST :
										ld_state = 0;
										ld_state |= ld_2d_data.ld_value << 24;
										ld_state |= ld_1d_data.ld_value << 16;	
										ld_state |= ld_1d_data.ld_on_off_state;																				
										ld_state_ack();	
									break;																																													
								default :
									printf("TCP_default\r\n");		
									break;
							}
							
						}
						#endif
					}
					//pthread_mutex_unlock(&mutex_lock);
					tcp_status = IDLE;
				}

			}
			else if ( poll_events_recv.revents & POLLERR)      // event 가 에러?
			{
				printf( "TCP0 Event Error!!!!\n");
				exit=0;
				break;
			}
		}
	}
	close(*(int *)client_p);
	client_status[0]=TCP_CLIENT_CLOSE;
	return 0;
}

void *RecvThread1(void *client_p)
{  
	TCP_PACKET packet;
	char exit=1;
	struct pollfd     poll_events_recv;      // 체크할 event 정보를 갖는 struct
   	int    poll_state_recv;
	char buff[BUFSIZE+5];
	int waitcnt;

	printf( "TCP1 CONNECT!!!!\n");
	 // poll 사용을 위한 준비
	poll_events_recv.fd        = (*(int *)client_p);
	//감시 대상인 디스크립터를 지정
	poll_events_recv.events    = POLLIN | POLLERR;          // 수신된 자료가 있는지, 에러가 있는지
	poll_events_recv.revents   = 0;	//revents를 0 으로 청소

	
	while(exit)
	{
		poll_state_recv = poll(                               // poll()을 호출하여 event 발생 여부 확인     
		(struct pollfd*)&poll_events_recv, // event 등록 변수
		1,  // 체크할 pollfd 개수
		1000   // time out 시간
		);
		if ( 0 < poll_state_recv)                             // 발생한 event 가 있음
		{     
			if ( poll_events_recv.revents & POLLIN)            // event 가 자료 수신?
			{
				if(recvn(*(int *)client_p,(char *)&packet,4,0)<=0) 
				{
				      printf( "TCP1 DISCONNECT!!!!\n");
				      exit=0;
				}
				else{
					/*waitcnt =0;
					while(waitcnt++< 100) {
						if(tcp_status == IDLE) break;
						usleep(1);
					}
					if(waitcnt == 100) printf("TCP1 wait over time\r\n");*/

					tcp_status = BUSY;
					
					if(tprf) printf("TCP1 : sig_id = %04x, len = %d\r\n",packet.sig_id,packet.len); 

					if(packet.sig_id == REQ_FILE_RCV) {
						file_receive(client_p,&packet,buff);
					}
					else {	

						if(packet.len>0) {
							memset(buff,0,BUFSIZE);
							recvn(*(int *)client_p,buff,packet.len,0);
							memcpy(&TcpRxQ[TcpRxPush].info[0], buff, packet.len);
						}
					    TcpRxQ[TcpRxPush].port_id = client_p;
					    TcpRxQ[TcpRxPush].sig_id = packet.sig_id;
//					    if(packet.sig_id == PatternId) t = myclock();
					    TcpRxQ[TcpRxPush].len = packet.len;
						TcpRxPush++;
						if(TcpRxPush == TCP_QSIZE) TcpRxPush = 0;
					}
////ysunlock
					tcp_status = IDLE;
				}

			}
			else if ( poll_events_recv.revents & POLLERR)      // event 가 에러?
			{
				printf( "TCP1 Event Error!!!!\n");
				exit=0;
				break;
			}
		}
	}
	close(*(int *)client_p);
	client_status[1]=TCP_CLIENT_CLOSE;
	return 0;
}

void *RecvThread2(void *client_p)
{  
	TCP_PACKET packet;
	char exit=1;
	struct pollfd     poll_events_recv;      // 체크할 event 정보를 갖는 struct
   	int    poll_state_recv;
	char buff[BUFSIZE+5];
	int waitcnt;

	printf( "TCP2 CONNECT!!!!\n");
	 // poll 사용을 위한 준비
	poll_events_recv.fd        = (*(int *)client_p);
	//감시 대상인 디스크립터를 지정
	poll_events_recv.events    = POLLIN | POLLERR;          // 수신된 자료가 있는지, 에러가 있는지
	poll_events_recv.revents   = 0;	//revents를 0 으로 청소

	
	while(exit)
	{
		poll_state_recv = poll(                               // poll()을 호출하여 event 발생 여부 확인     
		(struct pollfd*)&poll_events_recv, // event 등록 변수
									1,  // 체크할 pollfd 개수
									1000   // time out 시간
		);
		if ( 0 < poll_state_recv)                             // 발생한 event 가 있음
		{     
			if ( poll_events_recv.revents & POLLIN)            // event 가 자료 수신?
			{
				if(recvn(*(int *)client_p,(char *)&packet,4,0)<=0) 
				{
				      printf( "TCP2 DISCONNECT!!!!\n");
				      exit=0;
				}
				else{
					/*waitcnt =0;
					while(waitcnt++< 100) {
						if(tcp_status == IDLE) break;
						usleep(1);
					}
					if(waitcnt == 100) printf("TCP2 wait over time\r\n");*/

					tcp_status = BUSY;
					
					if(tprf) printf("TCP2 : sig_id = %04x, len = %d\r\n",packet.sig_id,packet.len); 

					if(packet.sig_id == REQ_FILE_RCV) {
						file_receive(client_p,&packet,buff);
					}
					else {	
						if(packet.len>0) {
							memset(buff,0,BUFSIZE);
							recvn(*(int *)client_p,buff,packet.len,0);
							memcpy(&TcpRxQ[TcpRxPush].info[0], buff, packet.len);
						}
						TcpRxQ[TcpRxPush].port_id = client_p;
						TcpRxQ[TcpRxPush].sig_id = packet.sig_id;
//					      if(packet.sig_id == PatternId) t = myclock();
						TcpRxQ[TcpRxPush].len = packet.len;
						TcpRxPush++;
						if(TcpRxPush == TCP_QSIZE) TcpRxPush = 0;
					}
////ysunlock
					tcp_status = IDLE;
				}

			}
			else if ( poll_events_recv.revents & POLLERR)      // event 가 에러?
			{
				printf( "TCP2 Event Error!!!!\n");
				exit=0;
				break;
			}
		}
	}
	close(*(int *)client_p);
	client_status[2]=TCP_CLIENT_CLOSE;
	return 0;
}

void file_receive(void *client_p,TCP_PACKET *packet ,char *buf)
{
	int retval,i;
	FILE *fp;

	char dir_path[FOLDER_NAME_SIZE];
	unsigned int totalbytes=0;
	unsigned int numtotal=0;
	
	//APRF = 1;

	memset(dir_path,0,256);
	printf("File Download Start\n");
	retval=recvn(*(int *)client_p,dir_path,256,0);
	
	printf("File Download path: %s\n",dir_path);
	retval=recvn(*(int *)client_p,(char *)&totalbytes,sizeof(totalbytes),0);
	printf("File Download total size: %d\n",totalbytes);
	fp=fopen(dir_path,"wb");
	if(fp==NULL){
	      perror("Error");
		// printf("file_open error\n");
	}
	else
	{
		while(1){
			
		      i=(totalbytes-numtotal);
		      
		      if(BUFSIZE>i) retval = recvn(*(int *)client_p,buf,i,0);
		      else retval = recvn(*(int *)client_p,buf,BUFSIZE,0);
		      
		      if(retval==SOCKET_ERROR)
		      {
			      printf("recv()\n");
			      break;
		      }
		      else if(retval == 0) {
		      	//printf("retval = 0\n");
		      	break;
		      }
		      else{
			      fwrite(buf,1,retval,fp);
			      if(ferror(fp)){
				      perror("Error");
				      //printf("write error\n");
				      break;
			      }
			      numtotal += retval;
		      }
		      
			 //printf("numtotal = %d, totalbytes = %d\n",numtotal,totalbytes);
		      
		      if(numtotal>=totalbytes)
		      {
				printf("File Download total size: %d recv size: %d\n",totalbytes,numtotal);
				break;
		      }
		}
		fclose(fp);
	}
	sync();
	//TcpAck(*(int *)client_p,REQ_FILE_RCV + 1);
	printf("File Download END\n");	
}

void data_ack(unsigned short sig)
{

	TCP_ACK_SEND packet;
	packet.sig_id = 0x0006;
	packet.len = 0x0006;
	packet.data = sig;
	//packet.data_size = count_data.trigger *24;	
	packet.data_size = count_data.trigger *40;	//40(x(8byte)+y(8byte)+z(8byte)+z_sum(8byte)+xy_sum(8byte))
	if(tprf) printf("SEND_DATA : sig_id = %04x, len = %d\r\n",packet.sig_id,packet.len); 
	send(client_sock[0],&packet,sizeof(packet),MSG_DONTWAIT|MSG_NOSIGNAL);	

	/*TCP_ACK_SEND packet;
	packet.sig_id = 0x0006;
	packet.len = 0x0006;
	packet.data = sig;
	if(formula_calculation_flag)	packet.data_size = count_data.trigger *24;	
	else	packet.data_size = count_data.trigger *6;
	if(tprf) printf("SEND_DATA : sig_id = %04x, len = %d\r\n",packet.sig_id,packet.len); 
	send(client_sock[0],&packet,sizeof(packet),MSG_DONTWAIT|MSG_NOSIGNAL);*/
}

void ver_ack()
{
	TCP_VER_ACK_SEND packet;
	//packet.sig_id = 0x0008;
	packet.sig_id = 0x000c;
	packet.len = 0x0002;
	packet.data = ep961_ver;
	if(tprf) printf("SEND_DATA : sig_id = %04x, len = %d\r\n",packet.sig_id,packet.len); 
	//send(*TcpRxQ[TcpRxPop].port_id,&packet,sizeof(packet),MSG_DONTWAIT|MSG_NOSIGNAL);
	send(client_sock[0],&packet,sizeof(packet),MSG_DONTWAIT|MSG_NOSIGNAL);
}

void ld_state_ack()
{
	TCP_VER_ACK_SEND packet;
	//packet.sig_id = 0x0008;
	packet.sig_id = LD_STATE_ACK;
	packet.len = 0x0004;
	packet.data = ld_state;
	if(tprf) printf("SEND_DATA : sig_id = %04x, len = %d\r\n",packet.sig_id,packet.len); 
	//send(*TcpRxQ[TcpRxPop].port_id,&packet,sizeof(packet),MSG_DONTWAIT|MSG_NOSIGNAL);
	send(client_sock[0],&packet,sizeof(packet),MSG_DONTWAIT|MSG_NOSIGNAL);
}

void data_send_ext(ADC_EXT *adc_data,int len,int pcnt,int ptcnt)
{
   int i;
   unsigned long long ullCheckSum = 0;
   unsigned long long ullSum = 0;
   unsigned long long ullData = 0;
   const int DATA_SIZE = sizeof(unsigned long long);
   TCP_SEND_DATA_FORMULA_EXT packet;
   memset(&packet,0,sizeof(packet));

   packet.sig_id = 0x00ff;
   packet.packet_count = pcnt;
   packet.packet_index = ptcnt;
   packet.len = len +2+2+8;
   memcpy(packet.info,adc_data,len);
   
   for(i=0 ; i < (len/8) ; i++) 
   {
      memcpy(&ullData, &packet.info[i], DATA_SIZE);
      ullSum += ullData;
   }
   ullCheckSum = ullSum & 0xffffffffffffffff;
   
   memcpy((packet.info+(len/8)), &ullCheckSum, DATA_SIZE);
   send(client_sock[0], &packet, packet.len+4, MSG_DONTWAIT|MSG_NOSIGNAL);		
}

