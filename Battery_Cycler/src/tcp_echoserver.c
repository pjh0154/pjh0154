/**
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of and a contribution to the lwIP TCP/IP stack.
 *
 * Credits go to Adam Dunkels (and the current maintainers) of this software.
 *
 * Christiaan Simons rewrote this file to get a more stable echo application.
 *
 **/

 /* This file was modified by ST */

#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "tcp_priv.h"
#include "tcp_echoserver.h"

#ifdef TCP_SERVER_ENABLE
#if LWIP_TCP

#define STX 0x02
#define ETX 0x03
unsigned char charge_discharge[100];
int count =0;

/* ECHO protocol states */
enum tcp_echoserver_states
{
  ES_NONE = 0,
  ES_ACCEPTED,
  ES_RECEIVED,
  ES_CLOSING
};

/* structure for maintaing connection infos to be passed as argument 
   to LwIP callbacks*/
struct tcp_echoserver_struct
{
  u8_t state;             /* current connection state */
  u8_t retries;
  struct tcp_pcb *pcb;    /* pointer on the current tcp_pcb */
  struct pbuf *p;         /* pointer on the received/to be transmitted pbuf */
};


static err_t tcp_echoserver_accept(void *arg, struct tcp_pcb *newpcb, err_t err);
static err_t tcp_echoserver_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void tcp_echoserver_error(void *arg, err_t err);
err_t tcp_echoserver_poll(void *arg, struct tcp_pcb *tpcb);
static err_t tcp_echoserver_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void tcp_echoserver_send(struct tcp_pcb *tpcb, struct tcp_echoserver_struct *es);
static void tcp_echoserver_connection_close(struct tcp_pcb *tpcb, struct tcp_echoserver_struct *es);
static void tcp_server_recv_function(struct tcp_pcb *tpcb, struct pbuf *p);
static void tcp_server_com_task(struct tcp_pcb *tpcb, char *ptr);

/**
  * @brief  Initializes the tcp echo server
  * @param  None
  * @retval None
  */
HAL_StatusTypeDef tcp_echoserver_init(void)
{
  HAL_StatusTypeDef result = HAL_ERROR;
  struct tcp_pcb *tcp_echoserver_pcb;

  /* create new tcp pcb */
  tcp_echoserver_pcb = tcp_new();

  if (tcp_echoserver_pcb != NULL)
  {
    err_t err;
    
    /* bind echo_pcb to port 7 (ECHO protocol) */
    err = tcp_bind(tcp_echoserver_pcb, IP_ADDR_ANY, TCP_ECHOSERVER_PORT);
    
    if (err == ERR_OK)
    {
      /* start tcp listening for echo_pcb */
      tcp_echoserver_pcb = tcp_listen(tcp_echoserver_pcb);
      
      /* initialize LwIP tcp_accept callback function */
      tcp_accept(tcp_echoserver_pcb, tcp_echoserver_accept);
      result = HAL_OK;

      printf("TCP Server Init OK\r\n");
    }
    else 
    {
      /* deallocate the pcb */
      memp_free(MEMP_TCP_PCB, tcp_echoserver_pcb);

      printf("TCP Server Init Fail\r\n");
    }
  }
  return result;
}

/**
  * @brief  This function is the implementation of tcp_accept LwIP callback
  * @param  arg: not used
  * @param  newpcb: pointer on tcp_pcb struct for the newly created tcp connection
  * @param  err: not used 
  * @retval err_t: error status
  */
static err_t tcp_echoserver_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  err_t ret_err;
  struct tcp_echoserver_struct *es;

  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(err);

  /* set priority for the newly accepted tcp connection newpcb */
  tcp_setprio(newpcb, TCP_PRIO_MIN);

  /* allocate structure es to maintain tcp connection informations */
  es = (struct tcp_echoserver_struct *)mem_malloc(sizeof(struct tcp_echoserver_struct));
  if (es != NULL)
  {
    es->state = ES_ACCEPTED;
    es->pcb = newpcb;
    es->retries = 0;
    es->p = NULL;
    
    /* pass newly allocated es structure as argument to newpcb */
    tcp_arg(newpcb, es);
    
    /* initialize lwip tcp_recv callback function for newpcb  */ 
    tcp_recv(newpcb, tcp_echoserver_recv);
    
    /* initialize lwip tcp_err callback function for newpcb  */
    tcp_err(newpcb, tcp_echoserver_error);
    
    /* initialize lwip tcp_poll callback function for newpcb */
    tcp_poll(newpcb, tcp_echoserver_poll, 0);

    ret_err = ERR_OK;
  }
  else
  {
    /*  close tcp connection */
    tcp_echoserver_connection_close(newpcb, es);
    /* return memory error */
    ret_err = ERR_MEM;
  }
  return ret_err;  
}


/**
  * @brief  This function is the implementation for tcp_recv LwIP callback
  * @param  arg: pointer on a argument for the tcp_pcb connection
  * @param  tpcb: pointer on the tcp_pcb connection
  * @param  pbuf: pointer on the received pbuf
  * @param  err: error information regarding the reveived pbuf
  * @retval err_t: error code
  */
static err_t tcp_echoserver_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  struct tcp_echoserver_struct *es;
  err_t ret_err;

  LWIP_ASSERT("arg != NULL",arg != NULL);

  es = (struct tcp_echoserver_struct *)arg;

  /* if we receive an empty tcp frame from client => close connection */
  if (p == NULL)
  {
	/* remote host closed connection */
	es->state = ES_CLOSING;
	if(es->p == NULL)
	{
	   /* we're done sending, close connection */
	   tcp_echoserver_connection_close(tpcb, es);
	}
	else
	{
	  /* we're not done yet */
	  /* acknowledge received packet */
	  tcp_sent(tpcb, tcp_echoserver_sent);

	  /* send remaining data*/
	  tcp_echoserver_send(tpcb, es);
	}
	ret_err = ERR_OK;
  }
  /* else : a non empty frame was received from client but for some reason err != ERR_OK */
  else if(err != ERR_OK)
  {
	/* free received pbuf*/
	if (p != NULL)
	{
	  es->p = NULL;
	  pbuf_free(p);
	}
	ret_err = err;
  }
  else if(es->state == ES_ACCEPTED)
  {
	/* initialize LwIP tcp_sent callback function */
	tcp_sent(tpcb, tcp_echoserver_sent);

	/* Acknowledge data reception */
	tcp_recved(tpcb, p->tot_len);

	tcp_server_recv_function(tpcb, p);

	pbuf_free(p);

	ret_err = ERR_OK;

	/* first data chunk in p->payload */
	//es->state = ES_RECEIVED;

	/* store reference to incoming pbuf (chain) */
	//es->p = p;

	/* initialize LwIP tcp_sent callback function */
	//tcp_sent(tpcb, tcp_echoserver_sent);

	/* send back the received data (echo) */
	//tcp_echoserver_send(tpcb, es);

	//ret_err = ERR_OK;
  }
  else if (es->state == ES_RECEIVED)
  {
	/* more data received from client and previous data has been already sent*/
	if(es->p == NULL)
	{
	  /* Acknowledge data reception */
	  tcp_recved(tpcb, p->tot_len);

	  tcp_server_recv_function(tpcb, p);

	  pbuf_free(p);

	  //es->p = p;

	  /* send back received data */
	  //tcp_echoserver_send(tpcb, es);
	}
	else
	{
	  struct pbuf *ptr;

	  /* chain pbufs to the end of what we recv'ed previously  */
	  ptr = es->p;
	  pbuf_chain(ptr,p);
	}
	ret_err = ERR_OK;
  }
  else if(es->state == ES_CLOSING)
  {
	/* odd case, remote side closing twice, trash data */
	tcp_recved(tpcb, p->tot_len);
	es->p = NULL;
	pbuf_free(p);
	ret_err = ERR_OK;
  }
  else
  {
	/* unkown es->state, trash data  */
	tcp_recved(tpcb, p->tot_len);
	es->p = NULL;
	pbuf_free(p);
	ret_err = ERR_OK;
  }
  return ret_err;
}

/**
  * @brief  This function implements the tcp_err callback function (called
  *         when a fatal tcp_connection error occurs. 
  * @param  arg: pointer on argument parameter 
  * @param  err: not used
  * @retval None
  */
static void tcp_echoserver_error(void *arg, err_t err)
{
  struct tcp_echoserver_struct *es;

  LWIP_UNUSED_ARG(err);

  es = (struct tcp_echoserver_struct *)arg;
  if (es != NULL)
  {
    /*  free es structure */
    mem_free(es);
  }
}

/**
  * @brief  This function implements the tcp_poll LwIP callback function
  * @param  arg: pointer on argument passed to callback
  * @param  tpcb: pointer on the tcp_pcb for the current tcp connection
  * @retval err_t: error code
  */
err_t tcp_echoserver_poll(void *arg, struct tcp_pcb *tpcb)
{
  err_t ret_err;
  struct tcp_echoserver_struct *es;

  es = (struct tcp_echoserver_struct *)arg;
  if (es != NULL)
  {
    if (es->p != NULL)
    {
      tcp_sent(tpcb, tcp_echoserver_sent);
      /* there is a remaining pbuf (chain) , try to send data */
      tcp_echoserver_send(tpcb, es);
    }
    else
    {
      /* no remaining pbuf (chain)  */
      if(es->state == ES_CLOSING)
      {
        /*  close tcp connection */
        tcp_echoserver_connection_close(tpcb, es);
      }
    }
    ret_err = ERR_OK;
  }
  else
  {
    /* nothing to be done */
    tcp_abort(tpcb);
    ret_err = ERR_ABRT;
  }
  return ret_err;
}

/**
  * @brief  This function implements the tcp_sent LwIP callback (called when ACK
  *         is received from remote host for sent data) 
  * @param  None
  * @retval None
  */
static err_t tcp_echoserver_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
  struct tcp_echoserver_struct *es;

  LWIP_UNUSED_ARG(len);

  es = (struct tcp_echoserver_struct *)arg;
  es->retries = 0;
  
  if(es->p != NULL)
  {
    /* still got pbufs to send */
    tcp_sent(tpcb, tcp_echoserver_sent);
    tcp_echoserver_send(tpcb, es);
  }
  else
  {
    /* if no more data to send and client closed connection*/
    if(es->state == ES_CLOSING)
      tcp_echoserver_connection_close(tpcb, es);
  }
  return ERR_OK;
}


/**
  * @brief  This function is used to send data for tcp connection
  * @param  tpcb: pointer on the tcp_pcb connection
  * @param  es: pointer on echo_state structure
  * @retval None
  */
static void tcp_echoserver_send(struct tcp_pcb *tpcb, struct tcp_echoserver_struct *es)
{
  struct pbuf *ptr;
  err_t wr_err = ERR_OK;
 
  while ((wr_err == ERR_OK) &&
         (es->p != NULL) && 
         (es->p->len <= tcp_sndbuf(tpcb)))
  {
    
    /* get pointer on pbuf from es structure */
    ptr = es->p;

    /* enqueue data for transmission */
    wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);
    
    if (wr_err == ERR_OK)
    {
      u16_t plen;
      u8_t freed;

      plen = ptr->len;
     
      /* continue with next pbuf in chain (if any) */
      es->p = ptr->next;
      
      if(es->p != NULL)
      {
        /* increment reference count for es->p */
        pbuf_ref(es->p);
      }
      
     /* chop first pbuf from chain */
      do
      {
        /* try hard to free pbuf */
        freed = pbuf_free(ptr);
      }
      while(freed == 0);
     /* we can read more data now */
     tcp_recved(tpcb, plen);
   }
   else if(wr_err == ERR_MEM)
   {
      /* we are low on memory, try later / harder, defer to poll */
     es->p = ptr;
   }
   else
   {
     /* other problem ?? */
   }
  }
}

/**
  * @brief  This functions closes the tcp connection
  * @param  tcp_pcb: pointer on the tcp connection
  * @param  es: pointer on echo_state structure
  * @retval None
  */
static void tcp_echoserver_connection_close(struct tcp_pcb *tpcb, struct tcp_echoserver_struct *es)
{
  
  /* remove all callbacks */
  tcp_arg(tpcb, NULL);
  tcp_sent(tpcb, NULL);
  tcp_recv(tpcb, NULL);
  tcp_err(tpcb, NULL);
  tcp_poll(tpcb, NULL, 0);
  
  /* delete es structure */
  if (es != NULL)
  {
    mem_free(es);
  }  
  
  /* close tcp connection */
  tcp_close(tpcb);
}

//unsigned int recv_cnt = 0;
static void tcp_server_recv_function(struct tcp_pcb *tpcb, struct pbuf *p)
{
	unsigned int i = 0;
	unsigned int recv_cnt = 0;
	char *ptr = p->payload;
	static char recv_data[TCP_SERVER_BUFFER_SIZE] = {0, };

	if(p->len < TCP_SERVER_BUFFER_SIZE)
	{
		for(i=0; i<p->len; i++)
		{
			if(*ptr == STX)
			{
				memset(recv_data, 0, sizeof(recv_data));
				recv_cnt = 0;
			}
			else if(*ptr == ETX)
			{
				tcp_server_com_task(tpcb, recv_data);
			}
			else
			{
				recv_data[recv_cnt++] = *ptr;
				if(recv_cnt >= TCP_SERVER_BUFFER_SIZE) recv_cnt = 0;
			}
			ptr++;
		}
	}
}

static void tcp_server_com_task(struct tcp_pcb *tpcb, char *ptr)
{
	if(!strncmp(ptr, "TEST", 4))
	{
		printf("TP1\r\n");
	}

	else if(!strncmp(ptr, "BURA", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value1[20] = {0};

		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value1[i-1] = atoi(temp_ptr);
			i++;
			if(i == 14) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}

		if(i == 5)
		{
			buck_boost[0].Run_Stop_1 = (unsigned char)temp_value1[0];
			buck_boost[0].Run_Stop_2 = (unsigned char)temp_value1[1];
			buck_boost[0].Run_Stop_3 = (unsigned char)temp_value1[2];
			buck_boost[0].Run_Stop_4 = (unsigned char)temp_value1[3];

			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x100, 0x000, (char *)&buck_boost[0].Run_Stop_1, 4) != HAL_OK) printf("ENQUE Fail\r\n");

/*			if(buck_boost[0].Run_Stop_1 == 1 || buck_boost[0].Run_Stop_2 == 1 || buck_boost[0].Run_Stop_3 == 1 || buck_boost[0].Run_Stop_4 ==1)
			{
						acdc.dc_voltage_r = 36768;
						printf("acdc on\r\n");
						manual_flag = 1;
						//HAL_Delay(5);
						acdc_ovp = 1;
			}
			else if(buck_boost[0].Run_Stop_1 == 0 && buck_boost[0].Run_Stop_2 == 0 && buck_boost[0].Run_Stop_3 == 0 && buck_boost[0].Run_Stop_4 == 0)
			{
				auto_off();
				acdc_ovp = 0;
				psfb_ovp = 0;
			}*/

		count=0;
			//if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x100, 0x000, (char *)&buck_boost[0].Run_Stop_1, 4) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\nbuck_boost[0].Run_Stop_1=               %d\r\n", buck_boost[0].Run_Stop_1);
				printf("buck_boost[0].Run_Stop_2=               %d\r\n", buck_boost[0].Run_Stop_2);
				printf("buck_boost[0].Run_Stop_3=               %d\r\n", buck_boost[0].Run_Stop_3);
				printf("buck_boost[0].Run_Stop_4=               %d\r\n", buck_boost[0].Run_Stop_4);
			}
		}
	}
	else if(!strncmp(ptr, "BURB", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value2[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value2[i-1] = atoi(temp_ptr);
			i++;
			if(i == 14) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}

		if(i == 5)
		{
			buck_boost[1].Run_Stop_1 = (unsigned char)temp_value2[0];
			buck_boost[1].Run_Stop_2 = (unsigned char)temp_value2[1];
			buck_boost[1].Run_Stop_3 = (unsigned char)temp_value2[2];
			buck_boost[1].Run_Stop_4 = (unsigned char)temp_value2[3];

			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x200, 0x000, (char *)&buck_boost[1].Run_Stop_1, 4) != HAL_OK) printf("ENQUE Fail\r\n");

/*			if(buck_boost[1].Run_Stop_1 == 1 || buck_boost[1].Run_Stop_2 == 1 || buck_boost[1].Run_Stop_3 == 1 || buck_boost[1].Run_Stop_4 ==1)
			{
						acdc.dc_voltage_r = 36768;
						printf("acdc on\r\n");
						manual_flag = 1;
						//HAL_Delay(5);
						acdc_ovp = 1;
			}
			else if(buck_boost[1].Run_Stop_1 == 0 && buck_boost[1].Run_Stop_2 == 0 && buck_boost[1].Run_Stop_3 == 0 && buck_boost[1].Run_Stop_4 == 0)
			{
				auto_off1();
				acdc_ovp = 0;
				psfb_ovp = 0;
			}
		}*/
		count=0;
			//if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x100, 0x000, (char *)&buck_boost[0].Run_Stop_1, 4) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\nbuck_boost[1].Run_Stop_1=               %d\r\n", buck_boost[1].Run_Stop_1);
				printf("buck_boost[1].Run_Stop_2=               %d\r\n", buck_boost[1].Run_Stop_2);
				printf("buck_boost[1].Run_Stop_3=               %d\r\n", buck_boost[1].Run_Stop_3);
				printf("buck_boost[1].Run_Stop_4=               %d\r\n", buck_boost[1].Run_Stop_4);
			}
		}
	}
	else if(!strncmp(ptr, "BURC", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value3[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value3[i-1] = atoi(temp_ptr);
			i++;
			if(i == 14) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}

		if(i == 5)
		{
			buck_boost[2].Run_Stop_1 = (unsigned char)temp_value3[0];
			buck_boost[2].Run_Stop_2 = (unsigned char)temp_value3[1];
			buck_boost[2].Run_Stop_3 = (unsigned char)temp_value3[2];
			buck_boost[2].Run_Stop_4 = (unsigned char)temp_value3[3];

			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x300, 0x000, (char *)&buck_boost[2].Run_Stop_1, 4) != HAL_OK) printf("ENQUE Fail\r\n");

/*			if(buck_boost[2].Run_Stop_1 == 1 || buck_boost[2].Run_Stop_2 == 1 || buck_boost[2].Run_Stop_3 == 1 || buck_boost[2].Run_Stop_4 ==1)
			{
						acdc.dc_voltage_r = 36768;
						printf("acdc on\r\n");
						manual_flag = 1;
						//HAL_Delay(5);
						acdc_ovp = 1;
			}
			else if(buck_boost[2].Run_Stop_1 == 0 && buck_boost[2].Run_Stop_2 == 0 && buck_boost[2].Run_Stop_3 == 0 && buck_boost[2].Run_Stop_4 == 0)
			{
				auto_off2();
				acdc_ovp = 0;
				psfb_ovp = 0;
			}*/

		count=0;
			//if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x100, 0x000, (char *)&buck_boost[0].Run_Stop_1, 4) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\nbuck_boost[2].Run_Stop_1=               %d\r\n", buck_boost[2].Run_Stop_1);
				printf("buck_boost[2].Run_Stop_2=               %d\r\n", buck_boost[2].Run_Stop_2);
				printf("buck_boost[2].Run_Stop_3=               %d\r\n", buck_boost[2].Run_Stop_3);
				printf("buck_boost[2].Run_Stop_4=               %d\r\n", buck_boost[2].Run_Stop_4);
			}
		}
	}
	else if(!strncmp(ptr, "BURD", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value4[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value4[i-1] = atoi(temp_ptr);
			i++;
			if(i == 14) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}

		if(i == 5)
		{
			buck_boost[3].Run_Stop_1 = (unsigned char)temp_value4[0];
			buck_boost[3].Run_Stop_2 = (unsigned char)temp_value4[1];
			buck_boost[3].Run_Stop_3 = (unsigned char)temp_value4[2];
			buck_boost[3].Run_Stop_4 = (unsigned char)temp_value4[3];

			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x400, 0x000, (char *)&buck_boost[3].Run_Stop_1, 4) != HAL_OK) printf("ENQUE Fail\r\n");

/*			if(buck_boost[3].Run_Stop_1 == 1 || buck_boost[3].Run_Stop_2 == 1 || buck_boost[3].Run_Stop_3 == 1 || buck_boost[3].Run_Stop_4 ==1)
			{
						acdc.dc_voltage_r = 36768;
						printf("acdc on\r\n");
						manual_flag = 1;
						//HAL_Delay(5);
						acdc_ovp = 1;
			}
			else if(buck_boost[3].Run_Stop_1 == 0 && buck_boost[3].Run_Stop_2 == 0 && buck_boost[3].Run_Stop_3 == 0 && buck_boost[3].Run_Stop_4 == 0)
			{
				auto_off3();
				acdc_ovp = 0;
				psfb_ovp = 0;
			}*/

		count=0;
			//if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x100, 0x000, (char *)&buck_boost[0].Run_Stop_1, 4) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\buck_boost[3].Run_Stop_1=               %d\r\n", buck_boost[3].Run_Stop_1);
				printf("buck_boost[3].Run_Stop_2=               %d\r\n", buck_boost[3].Run_Stop_2);
				printf("buck_boost[3].Run_Stop_3=               %d\r\n", buck_boost[3].Run_Stop_3);
				printf("buck_boost[3].Run_Stop_4=               %d\r\n", buck_boost[3].Run_Stop_4);
			}
		}
	}
	else if(!strncmp(ptr, "BCHA", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value5[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value5[i-1] = atoi(temp_ptr);
			i++;
			if(i == 14) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}

		if(i == 14)
		{
			buck_boost[0].Cycle_SetRepeat = (unsigned char)temp_value5[0];
			buck_boost[0].step_SetRepeat = (unsigned char)temp_value5[1];
			buck_boost[0].SOC_SetTest = (unsigned short)temp_value5[2];
			buck_boost[0].step_channel = (unsigned char)temp_value5[3];
			buck_boost[0].step_step_id = (unsigned char)temp_value5[4];
			buck_boost[0].norminal_voltage = (unsigned short)temp_value5[5];
			buck_boost[0].norminal_capacity = (unsigned short)temp_value5[6];
			buck_boost[0].max_voltage = (unsigned short)temp_value5[7];
			buck_boost[0].min_voltage = (unsigned short)temp_value5[8];
			buck_boost[0].max_current = (unsigned short)temp_value5[9];
			buck_boost[0].min_current = (unsigned short)temp_value5[10];
			buck_boost[0].max_soc = (unsigned short)temp_value5[11];
			buck_boost[0].min_soc = (unsigned short)temp_value5[12];

			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x103, 0x000, (char *)&buck_boost[0].step_channel, 2) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x104, 0x000, (char *)&buck_boost[0].norminal_voltage, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x105, 0x000, (char *)&buck_boost[0].max_current, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x101, 0x000, (char *)&buck_boost[0].Cycle_SetRepeat, 2) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x102, 0x000, (char *)&buck_boost[0].SOC_SetTest, 2) != HAL_OK) printf("ENQUE Fail\r\n");

			if(cprf)
			{
				printf("\r\nbuck_boost[0].Cycle_SetRepeat=               %d\r\n", buck_boost[0].Cycle_SetRepeat);
				printf("buck_boost[0].step_SetRepeat=                 %d\r\n", buck_boost[0].step_SetRepeat);
				printf("buck_boost[0].SOC_SetTest=                    %d\r\n", buck_boost[0].SOC_SetTest);
				printf("buck_boost[0].step_channel=                   %d\r\n", buck_boost[0].step_channel);
				printf("buck_boost[0].step_step_id=                   %d\r\n", buck_boost[0].step_step_id);
				printf("buck_boost[0].norminal_voltage=               %d\r\n", buck_boost[0].norminal_voltage);
				printf("buck_boost[0].norminal_capacity=              %d\r\n", buck_boost[0].norminal_capacity);
				printf("buck_boost[0].max_voltage=                    %d\r\n", buck_boost[0].max_voltage);
				printf("buck_boost[0].min_voltage=                    %d\r\n", buck_boost[0].min_voltage);
				printf("buck_boost[0].max_current=                    %d\r\n", buck_boost[0].max_current);
				printf("buck_boost[0].min_current=                    %d\r\n", buck_boost[0].min_current);
				printf("buck_boost[0].max_soc=                        %d\r\n", buck_boost[0].max_soc);
				printf("buck_boost[0].min_soc=                        %d\r\n", buck_boost[0].min_soc);
			}
		}
	}
	else if(!strncmp(ptr, "BCHB", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value6[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value6[i-1] = atoi(temp_ptr);
			i++;
			if(i == 14) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}

		if(i == 14)
		{
			buck_boost[1].Cycle_SetRepeat = (unsigned char)temp_value6[0];
			buck_boost[1].step_SetRepeat = (unsigned char)temp_value6[1];
			buck_boost[1].SOC_SetTest = (unsigned short)temp_value6[2];
			buck_boost[1].step_channel = (unsigned char)temp_value6[3];
			buck_boost[1].step_step_id = (unsigned char)temp_value6[4];
			buck_boost[1].norminal_voltage = (unsigned short)temp_value6[5];
			buck_boost[1].norminal_capacity = (unsigned short)temp_value6[6];
			buck_boost[1].max_voltage = (unsigned short)temp_value6[7];
			buck_boost[1].min_voltage = (unsigned short)temp_value6[8];
			buck_boost[1].max_current = (unsigned short)temp_value6[9];
			buck_boost[1].min_current = (unsigned short)temp_value6[10];
			buck_boost[1].max_soc = (unsigned short)temp_value6[11];
			buck_boost[1].min_soc = (unsigned short)temp_value6[12];

			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x203, 0x000, (char *)&buck_boost[1].step_channel, 2) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x204, 0x000, (char *)&buck_boost[1].norminal_voltage, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x205, 0x000, (char *)&buck_boost[1].max_current, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x201, 0x000, (char *)&buck_boost[1].Cycle_SetRepeat, 2) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x202, 0x000, (char *)&buck_boost[1].SOC_SetTest, 2) != HAL_OK) printf("ENQUE Fail\r\n");

			if(cprf)
			{
				printf("\r\nbuck_boost[1].Cycle_SetRepeat=               %d\r\n", buck_boost[1].Cycle_SetRepeat);
				printf("buck_boost[1].step_SetRepeat=                 %d\r\n", buck_boost[1].step_SetRepeat);
				printf("buck_boost[1].SOC_SetTest=                    %d\r\n", buck_boost[1].SOC_SetTest);
				printf("buck_boost[1].step_channel=                   %d\r\n", buck_boost[1].step_channel);
				printf("buck_boost[1].step_step_id=                   %d\r\n", buck_boost[1].step_step_id);
				printf("buck_boost[1].norminal_voltage=               %d\r\n", buck_boost[1].norminal_voltage);
				printf("buck_boost[1].norminal_capacity=              %d\r\n", buck_boost[1].norminal_capacity);
				printf("buck_boost[1].max_voltage=                    %d\r\n", buck_boost[1].max_voltage);
				printf("buck_boost[1].min_voltage=                    %d\r\n", buck_boost[1].min_voltage);
				printf("buck_boost[1].max_current=                    %d\r\n", buck_boost[1].max_current);
				printf("buck_boost[1].min_current=                    %d\r\n", buck_boost[1].min_current);
				printf("buck_boost[1].max_soc=                        %d\r\n", buck_boost[1].max_soc);
				printf("buck_boost[1].min_soc=                        %d\r\n", buck_boost[1].min_soc);
			}
		}
	}
	else if(!strncmp(ptr, "BCHC", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value7[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value7[i-1] = atoi(temp_ptr);
			i++;
			if(i == 14) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 14)
		{
			buck_boost[2].Cycle_SetRepeat = (unsigned char)temp_value7[0];
			buck_boost[2].step_SetRepeat = (unsigned char)temp_value7[1];
			buck_boost[2].SOC_SetTest = (unsigned short)temp_value7[2];
			buck_boost[2].step_channel = (unsigned char)temp_value7[3];
			buck_boost[2].step_step_id = (unsigned char)temp_value7[4];
			buck_boost[2].norminal_voltage = (unsigned short)temp_value7[5];
			buck_boost[2].norminal_capacity = (unsigned short)temp_value7[6];
			buck_boost[2].max_voltage = (unsigned short)temp_value7[7];
			buck_boost[2].min_voltage = (unsigned short)temp_value7[8];
			buck_boost[2].max_current = (unsigned short)temp_value7[9];
			buck_boost[2].min_current = (unsigned short)temp_value7[10];
			buck_boost[2].max_soc = (unsigned short)temp_value7[11];
			buck_boost[2].min_soc = (unsigned short)temp_value7[12];

			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x303, 0x000, (char *)&buck_boost[2].step_channel, 2) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x304, 0x000, (char *)&buck_boost[2].norminal_voltage, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x305, 0x000, (char *)&buck_boost[2].max_current, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x301, 0x000, (char *)&buck_boost[2].Cycle_SetRepeat, 2) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x302, 0x000, (char *)&buck_boost[2].SOC_SetTest, 2) != HAL_OK) printf("ENQUE Fail\r\n");

			if(cprf)
			{
				printf("\r\nbuck_boost[2].Cycle_SetRepeat=               %d\r\n", buck_boost[2].Cycle_SetRepeat);
				printf("buck_boost[2].step_SetRepeat=                 %d\r\n", buck_boost[2].step_SetRepeat);
				printf("buck_boost[2].SOC_SetTest=                    %d\r\n", buck_boost[2].SOC_SetTest);
				printf("buck_boost[2].step_channel=                   %d\r\n", buck_boost[2].step_channel);
				printf("buck_boost[2].step_step_id=                   %d\r\n", buck_boost[2].step_step_id);
				printf("buck_boost[2].norminal_voltage=               %d\r\n", buck_boost[2].norminal_voltage);
				printf("buck_boost[2].norminal_capacity=              %d\r\n", buck_boost[2].norminal_capacity);
				printf("buck_boost[2].max_voltage=                    %d\r\n", buck_boost[2].max_voltage);
				printf("buck_boost[2].min_voltage=                    %d\r\n", buck_boost[2].min_voltage);
				printf("buck_boost[2].max_current=                    %d\r\n", buck_boost[2].max_current);
				printf("buck_boost[2].min_current=                    %d\r\n", buck_boost[2].min_current);
				printf("buck_boost[2].max_soc=                        %d\r\n", buck_boost[2].max_soc);
				printf("buck_boost[2].min_soc=                        %d\r\n", buck_boost[2].min_soc);
			}
		}
	}
	else if(!strncmp(ptr, "BCHD", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value8[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value8[i-1] = atoi(temp_ptr);
			i++;
			if(i == 14) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 14)
		{
			buck_boost[3].Cycle_SetRepeat = (unsigned char)temp_value8[0];
			buck_boost[3].step_SetRepeat = (unsigned char)temp_value8[1];
			buck_boost[3].SOC_SetTest = (unsigned short)temp_value8[2];
			buck_boost[3].step_channel = (unsigned char)temp_value8[3];
			buck_boost[3].step_step_id = (unsigned char)temp_value8[4];
			buck_boost[3].norminal_voltage = (unsigned short)temp_value8[5];
			buck_boost[3].norminal_capacity = (unsigned short)temp_value8[6];
			buck_boost[3].max_voltage = (unsigned short)temp_value8[7];
			buck_boost[3].min_voltage = (unsigned short)temp_value8[8];
			buck_boost[3].max_current = (unsigned short)temp_value8[9];
			buck_boost[3].min_current = (unsigned short)temp_value8[10];
			buck_boost[3].max_soc = (unsigned short)temp_value8[11];
			buck_boost[3].min_soc = (unsigned short)temp_value8[12];

			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x403, 0x000, (char *)&buck_boost[3].step_channel, 2) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x404, 0x000, (char *)&buck_boost[3].norminal_voltage, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x405, 0x000, (char *)&buck_boost[3].max_current, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x401, 0x000, (char *)&buck_boost[3].Cycle_SetRepeat, 2) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x402, 0x000, (char *)&buck_boost[3].SOC_SetTest, 2) != HAL_OK) printf("ENQUE Fail\r\n");

			if(cprf)
			{
				printf("\r\nbuck_boost[3].Cycle_SetRepeat=               %d\r\n", buck_boost[0].Cycle_SetRepeat);
				printf("buck_boost[3].step_SetRepeat=                 %d\r\n", buck_boost[0].step_SetRepeat);
				printf("buck_boost[3].SOC_SetTest=                    %d\r\n", buck_boost[0].SOC_SetTest);
				printf("buck_boost[3].step_channel=                   %d\r\n", buck_boost[0].step_channel);
				printf("buck_boost[3].step_step_id=                   %d\r\n", buck_boost[0].step_step_id);
				printf("buck_boost[3].norminal_voltage=               %d\r\n", buck_boost[0].norminal_voltage);
				printf("buck_boost[3].norminal_capacity=              %d\r\n", buck_boost[0].norminal_capacity);
				printf("buck_boost[3].max_voltage=                    %d\r\n", buck_boost[0].max_voltage);
				printf("buck_boost[3].min_voltage=                    %d\r\n", buck_boost[0].min_voltage);
				printf("buck_boost[3].max_current=                    %d\r\n", buck_boost[0].max_current);
				printf("buck_boost[3].min_current=                    %d\r\n", buck_boost[0].min_current);
				printf("buck_boost[3].max_soc=                        %d\r\n", buck_boost[0].max_soc);
				printf("buck_boost[3].min_soc=                        %d\r\n", buck_boost[0].min_soc);
			}
		}
	}
	else if(!strncmp(ptr, "BUSA", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value9[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value9[i-1] = atoi(temp_ptr);
			i++;
			if(i == 14) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 14)
		{
			buck_boost[0].step_channel = (unsigned char)temp_value9[0];
			buck_boost[0].step_step_id = (unsigned char)temp_value9[1];
			buck_boost[0].step_mode_id = (unsigned char)temp_value9[2];
			buck_boost[0].step_type_id = (unsigned char)temp_value9[3];
			buck_boost[0].step_voltage = (unsigned short)temp_value9[4];
			buck_boost[0].step_current = (unsigned short)temp_value9[5];
			buck_boost[0].step_time = (unsigned short)temp_value9[6];
			buck_boost[0].step_extra = (unsigned short)temp_value9[7];
			buck_boost[0].step_termination_voltage = (unsigned short)temp_value9[8];
			buck_boost[0].step_termination_current = (unsigned short)temp_value9[9];
			buck_boost[0].step_termination_soc = (unsigned short)temp_value9[10];
			buck_boost[0].step_termination_time = (unsigned short)temp_value9[11];
			buck_boost[0].step_termination_msec = (unsigned short)temp_value9[12];

			if(count < buck_boost[0].step_SetRepeat)
			{
				charge_discharge[count] = buck_boost[0].step_type_id;
				//printf("TP%d=%d\r\n",count,charge_discharge[count]);
				count +=1;
				if(count == buck_boost[0].step_SetRepeat) count = 0;
			}


			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x103, 0x000, (char *)&buck_boost[0].step_channel, 2) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x106, 0x000, (char *)&buck_boost[0].step_mode_id, 6) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x107, 0x000, (char *)&buck_boost[0].step_time, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x108, 0x000, (char *)&buck_boost[0].step_termination_soc, 6) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\nbuck_boost[0].step_channel=                  %d\r\n", buck_boost[0].step_channel);
				printf("buck_boost[0].step_step_id=                   %d\r\n", buck_boost[0].step_step_id);
				printf("buck_boost[0].step_mode_id=                   %d\r\n", buck_boost[0].step_mode_id);
				printf("buck_boost[0].step_type_id=                   %d\r\n", buck_boost[0].step_type_id);
				printf("buck_boost[0].step_voltage=                   %d\r\n", buck_boost[0].step_voltage);
				printf("buck_boost[0].norminal_voltage=               %d\r\n", buck_boost[0].norminal_voltage);
				printf("buck_boost[0].step_current=                   %d\r\n", buck_boost[0].step_current);
				printf("buck_boost[0].step_time=                      %d\r\n", buck_boost[0].step_time);
				printf("buck_boost[0].step_extra=                     %d\r\n", buck_boost[0].step_extra);
				printf("buck_boost[0].step_termination_voltage=       %d\r\n", buck_boost[0].step_termination_voltage);
				printf("buck_boost[0].step_termination_current=       %d\r\n", buck_boost[0].step_termination_current);
				printf("buck_boost[0].step_termination_soc=           %d\r\n", buck_boost[0].step_termination_soc);
				printf("buck_boost[0].step_termination_time=          %d\r\n", buck_boost[0].step_termination_time);
				printf("buck_boost[0].step_termination_msec=          %d\r\n", buck_boost[0].step_termination_msec);
			}
		}
	}
	else if(!strncmp(ptr, "BUSB", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value10[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value10[i-1] = atoi(temp_ptr);
			i++;
			if(i == 14) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 14)
		{
			buck_boost[1].step_channel = (unsigned char)temp_value10[0];
			buck_boost[1].step_step_id = (unsigned char)temp_value10[1];
			buck_boost[1].step_mode_id = (unsigned char)temp_value10[2];
			buck_boost[1].step_type_id = (unsigned char)temp_value10[3];
			buck_boost[1].step_voltage = (unsigned short)temp_value10[4];
			buck_boost[1].step_current = (unsigned short)temp_value10[5];
			buck_boost[1].step_time = (unsigned short)temp_value10[6];
			buck_boost[1].step_extra = (unsigned short)temp_value10[7];
			buck_boost[1].step_termination_voltage = (unsigned short)temp_value10[8];
			buck_boost[1].step_termination_current = (unsigned short)temp_value10[9];
			buck_boost[1].step_termination_soc = (unsigned short)temp_value10[10];
			buck_boost[1].step_termination_time = (unsigned short)temp_value10[11];
			buck_boost[1].step_termination_msec = (unsigned short)temp_value10[12];

			if(count < buck_boost[1].step_SetRepeat)
			{
				charge_discharge[count] = buck_boost[0].step_type_id;
				count +=1;
				if(count == buck_boost[1].step_SetRepeat) count = 0;
			}


			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x203, 0x000, (char *)&buck_boost[1].step_channel, 2) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x206, 0x000, (char *)&buck_boost[1].step_mode_id, 6) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x207, 0x000, (char *)&buck_boost[1].step_time, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x208, 0x000, (char *)&buck_boost[1].step_termination_soc, 6) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\nbuck_boost[1].step_channel=                  %d\r\n", buck_boost[1].step_channel);
				printf("buck_boost[1].step_step_id=                   %d\r\n", buck_boost[1].step_step_id);
				printf("buck_boost[1].step_mode_id=                   %d\r\n", buck_boost[1].step_mode_id);
				printf("buck_boost[1].step_type_id=                   %d\r\n", buck_boost[0].step_type_id);
				printf("buck_boost[1].step_voltage=                   %d\r\n", buck_boost[1].step_voltage);
				printf("buck_boost[1].norminal_voltage=               %d\r\n", buck_boost[1].norminal_voltage);
				printf("buck_boost[1].step_current=                   %d\r\n", buck_boost[1].step_current);
				printf("buck_boost[1].step_time=                      %d\r\n", buck_boost[1].step_time);
				printf("buck_boost[1].step_extra=                     %d\r\n", buck_boost[1].step_extra);
				printf("buck_boost[1].step_termination_voltage=       %d\r\n", buck_boost[1].step_termination_voltage);
				printf("buck_boost[1].step_termination_current=       %d\r\n", buck_boost[1].step_termination_current);
				printf("buck_boost[1].step_termination_soc=           %d\r\n", buck_boost[1].step_termination_soc);
				printf("buck_boost[1].step_termination_time=          %d\r\n", buck_boost[1].step_termination_time);
				printf("buck_boost[1].step_termination_msec=          %d\r\n", buck_boost[1].step_termination_msec);
			}
		}
	}
	else if(!strncmp(ptr, "BUSC", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value11[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value11[i-1] = atoi(temp_ptr);
			i++;
			if(i == 14) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 14)
		{
			buck_boost[2].step_channel = (unsigned char)temp_value11[0];
			buck_boost[2].step_step_id = (unsigned char)temp_value11[1];
			buck_boost[2].step_mode_id = (unsigned char)temp_value11[2];
			buck_boost[2].step_type_id = (unsigned char)temp_value11[3];
			buck_boost[2].step_voltage = (unsigned short)temp_value11[4];
			buck_boost[2].step_current = (unsigned short)temp_value11[5];
			buck_boost[2].step_time = (unsigned short)temp_value11[6];
			buck_boost[2].step_extra = (unsigned short)temp_value11[7];
			buck_boost[2].step_termination_voltage = (unsigned short)temp_value11[8];
			buck_boost[2].step_termination_current = (unsigned short)temp_value11[9];
			buck_boost[2].step_termination_soc = (unsigned short)temp_value11[10];
			buck_boost[2].step_termination_time = (unsigned short)temp_value11[11];
			buck_boost[2].step_termination_msec = (unsigned short)temp_value11[12];

			if(count < buck_boost[2].step_SetRepeat)
			{
				charge_discharge[count] = buck_boost[2].step_type_id;
				printf("TP%d=%d\r\n",count,charge_discharge[count]);
				count +=1;
				if(count == buck_boost[2].step_SetRepeat) count = 0;
			}


			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x303, 0x000, (char *)&buck_boost[2].step_channel, 2) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x306, 0x000, (char *)&buck_boost[2].step_mode_id, 6) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x307, 0x000, (char *)&buck_boost[2].step_time, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x308, 0x000, (char *)&buck_boost[2].step_termination_soc, 6) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\nbuck_boost[2].step_channel=                  %d\r\n", buck_boost[2].step_channel);
				printf("buck_boost[2].step_step_id=                   %d\r\n", buck_boost[2].step_step_id);
				printf("buck_boost[2].step_mode_id=                   %d\r\n", buck_boost[2].step_mode_id);
				printf("buck_boost[2].step_type_id=                   %d\r\n", buck_boost[2].step_type_id);
				printf("buck_boost[2].step_voltage=                   %d\r\n", buck_boost[2].step_voltage);
				printf("buck_boost[2].norminal_voltage=               %d\r\n", buck_boost[2].norminal_voltage);
				printf("buck_boost[2].step_current=                   %d\r\n", buck_boost[2].step_current);
				printf("buck_boost[2].step_time=                      %d\r\n", buck_boost[2].step_time);
				printf("buck_boost[2].step_extra=                     %d\r\n", buck_boost[2].step_extra);
				printf("buck_boost[2].step_termination_voltage=       %d\r\n", buck_boost[2].step_termination_voltage);
				printf("buck_boost[2].step_termination_current=       %d\r\n", buck_boost[2].step_termination_current);
				printf("buck_boost[2].step_termination_soc=           %d\r\n", buck_boost[2].step_termination_soc);
				printf("buck_boost[2].step_termination_time=          %d\r\n", buck_boost[2].step_termination_time);
				printf("buck_boost[2].step_termination_msec=          %d\r\n", buck_boost[2].step_termination_msec);
			}
		}
	}
	else if(!strncmp(ptr, "BUSD", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value12[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value12[i-1] = atoi(temp_ptr);
			i++;
			if(i == 14) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 14)
		{
			buck_boost[3].step_channel = (unsigned char)temp_value12[0];
			buck_boost[3].step_step_id = (unsigned char)temp_value12[1];
			buck_boost[3].step_mode_id = (unsigned char)temp_value12[2];
			buck_boost[3].step_type_id = (unsigned char)temp_value12[3];
			buck_boost[3].step_voltage = (unsigned short)temp_value12[4];
			buck_boost[3].step_current = (unsigned short)temp_value12[5];
			buck_boost[3].step_time = (unsigned short)temp_value12[6];
			buck_boost[3].step_extra = (unsigned short)temp_value12[7];
			buck_boost[3].step_termination_voltage = (unsigned short)temp_value12[8];
			buck_boost[3].step_termination_current = (unsigned short)temp_value12[9];
			buck_boost[3].step_termination_soc = (unsigned short)temp_value12[10];
			buck_boost[3].step_termination_time = (unsigned short)temp_value12[11];
			buck_boost[3].step_termination_msec = (unsigned short)temp_value12[12];

			if(count < buck_boost[3].step_SetRepeat)
			{
				charge_discharge[count] = buck_boost[3].step_type_id;
				printf("TP%d=%d\r\n",count,charge_discharge[count]);
				count +=1;
				if(count == buck_boost[3].step_SetRepeat) count = 0;
			}


			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x403, 0x000, (char *)&buck_boost[3].step_channel, 2) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x406, 0x000, (char *)&buck_boost[3].step_mode_id, 6) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x407, 0x000, (char *)&buck_boost[3].step_time, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x408, 0x000, (char *)&buck_boost[3].step_termination_soc, 6) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\nbuck_boost[3].step_channel=                  %d\r\n", buck_boost[3].step_channel);
				printf("buck_boost[3].step_step_id=                   %d\r\n", buck_boost[3].step_step_id);
				printf("buck_boost[3].step_mode_id=                   %d\r\n", buck_boost[3].step_mode_id);
				printf("buck_boost[3].step_type_id=                   %d\r\n", buck_boost[3].step_type_id);
				printf("buck_boost[3].step_voltage=                   %d\r\n", buck_boost[3].step_voltage);
				printf("buck_boost[3].norminal_voltage=               %d\r\n", buck_boost[3].norminal_voltage);
				printf("buck_boost[3].step_current=                   %d\r\n", buck_boost[3].step_current);
				printf("buck_boost[3].step_time=                      %d\r\n", buck_boost[3].step_time);
				printf("buck_boost[3].step_extra=                     %d\r\n", buck_boost[3].step_extra);
				printf("buck_boost[3].step_termination_voltage=       %d\r\n", buck_boost[3].step_termination_voltage);
				printf("buck_boost[3].step_termination_current=       %d\r\n", buck_boost[3].step_termination_current);
				printf("buck_boost[3].step_termination_soc=           %d\r\n", buck_boost[3].step_termination_soc);
				printf("buck_boost[3].step_termination_time=          %d\r\n", buck_boost[3].step_termination_time);
				printf("buck_boost[3].step_termination_msec=          %d\r\n", buck_boost[3].step_termination_msec);
			}
		}
	}

	else if(!strncmp(ptr, "ADRS", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value13[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value13[i-1] = (unsigned short)(strtol(temp_ptr, NULL, 16));
			i++;
			if(i == 14) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}

		if(i == 2)
		{
			acdc.run_stop = (unsigned char)temp_value13[0];
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x17, 0x000, (char *)&acdc.run_stop, 1) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\nrun_stop=             %d\r\n", acdc.run_stop);
			}
		}
	}
	else if(!strncmp(ptr, "ADP0", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value14[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value14[i-1] = atoi(temp_ptr);
			i++;
			if(i == 5) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 5)
		{
			acdc.dc_voltage_w = (unsigned short)temp_value14[0];
			acdc.ac_voltage = (unsigned short)temp_value14[1];
			acdc.ac_frequency_w = (unsigned short)temp_value14[2];
			acdc.pwm_frequency = (unsigned short)temp_value14[3];
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x10, 0x000, (char *)&acdc.dc_voltage_w, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\ndc_voltage_w=               %d\r\n", acdc.dc_voltage_w);
				printf("ac_voltage=                 %d\r\n", acdc.ac_voltage);
				printf("ac_frequency_w=             %d\r\n", acdc.ac_frequency_w);
				printf("pwm_frequency=              %d\r\n", acdc.pwm_frequency);
			}
		}

	}
	else if(!strncmp(ptr, "ADP1", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value15[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value15[i-1] = atoi(temp_ptr);
			i++;
			if(i == 5) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 5)
		{
			acdc.max_active_power = (unsigned short)temp_value15[0];
			acdc.max_ac_current = (unsigned short)temp_value15[1];
			acdc.max_ac_voltage = (unsigned short)temp_value15[2];
			acdc.min_ac_voltage = (unsigned short)temp_value15[3];
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x11, 0x000, (char *)&acdc.max_active_power, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\nmax_active_power=           %d\r\n", acdc.max_active_power);
				printf("max_ac_current=             %d\r\n", acdc.max_ac_current);
				printf("max_ac_voltage=             %d\r\n", acdc.max_ac_voltage);
				printf("min_ac_voltage=             %d\r\n", acdc.min_ac_voltage);
			}
		}
	}
	else if(!strncmp(ptr, "ADP2", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value16[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value16[i-1] = atoi(temp_ptr);
			i++;
			if(i == 5) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 5)
		{
			acdc.max_ac_frequency = (unsigned short)temp_value16[0];
			acdc.min_ac_frequency = (unsigned short)temp_value16[1];
			acdc.max_dc_voltage = (unsigned short)temp_value16[2];
			acdc.max_dc_current = (unsigned short)temp_value16[3];
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x12, 0x000, (char *)&acdc.max_ac_frequency, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\nmax_ac_frequency=              %d\r\n", acdc.max_ac_frequency);
				printf("min_ac_frequency=             %d\r\n", acdc.min_ac_frequency);
				printf("max_dc_voltage=               %d\r\n", acdc.max_dc_voltage);
				printf("max_dc_current=               %d\r\n", acdc.max_dc_current);
			}
		}
	}
	else if(!strncmp(ptr, "ADP3", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value17[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value17[i-1] = atoi(temp_ptr);
			i++;
			if(i == 5) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 5)
		{
			acdc.dc_start_voltage = (unsigned short)temp_value17[0];
			acdc.dc_stop_voltage = (unsigned short)temp_value17[1];
			acdc.fan_start_temp = (unsigned short)temp_value17[2];
			acdc.fan_stop_temp = (unsigned short)temp_value17[3];
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x13, 0x000, (char *)&acdc.dc_start_voltage, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\ndc_start_voltage=             %d\r\n", acdc.dc_start_voltage);
				printf("dc_stop_voltage=              %d\r\n", acdc.dc_stop_voltage);
				printf("fan_start_temp=               %d\r\n", acdc.fan_start_temp);
				printf("fan_stop_temp=                %d\r\n", acdc.fan_stop_temp);
			}
		}
	}
	else if(!strncmp(ptr, "ADP4", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value18[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value18[i-1] = atoi(temp_ptr);
			i++;
			if(i == 5) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 5)
		{
			acdc.step1_v = (unsigned short)temp_value18[0];
			acdc.step1_p = (unsigned short)temp_value18[1];
			acdc.step2_v = (unsigned short)temp_value18[2];
			acdc.step2_p = (unsigned short)temp_value18[3];
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x14, 0x000, (char *)&acdc.step1_v, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\nstep1_v=                      %d\r\n", acdc.step1_v);
				printf("step1_p=                      %d\r\n", acdc.step1_p);
				printf("step2_v=                      %d\r\n", acdc.step2_v);
				printf("step2_p=                      %d\r\n", acdc.step2_p);
			}
		}
	}
	else if(!strncmp(ptr, "ADP5", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value19[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value19[i-1] = atoi(temp_ptr);
			i++;
			if(i == 5) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 5)
		{
			acdc.step3_v = (unsigned short)temp_value19[0];
			acdc.step3_p = (unsigned short)temp_value19[1];
			acdc.step4_v = (unsigned short)temp_value19[2];
			acdc.step4_p = (unsigned short)temp_value19[3];
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x15, 0x000, (char *)&acdc.step3_v, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\nstep3_v=                      %d\r\n", acdc.step3_v);
				printf("step3_p=                      %d\r\n", acdc.step3_p);
				printf("step4_v=                      %d\r\n", acdc.step4_v);
				printf("step4_p=                      %d\r\n", acdc.step4_p);
			}
		}
	}
	else if(!strncmp(ptr, "ADP6", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value20[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value20[i-1] = atoi(temp_ptr);
			i++;
			if(i == 5) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 5)
		{
			acdc.step5_v = (unsigned short)temp_value20[0];
			acdc.step5_p = (unsigned short)temp_value20[1];
			acdc.memory_key = (unsigned short)temp_value20[2];
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x16, 0x000, (char *)&acdc.step5_v, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\nstep5_v=                      %d\r\n", acdc.step5_v);
				printf("step5_p=                      %d\r\n", acdc.step5_p);
				printf("memory_key=                   %d\r\n", acdc.memory_key);
			}
		}
	}
	else if(!strncmp(ptr, "ADTE", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value21[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value21[i-1] = atoi(temp_ptr);
			i++;
			if(i == 3) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 3)
		{
			acdc.test_power = (unsigned short)temp_value21[0];
			acdc.pre_test_pwm_duty = (unsigned short)temp_value21[1];
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x18, 0x000, (char *)&acdc.test_power, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\ntest_power=                      %d\r\n", acdc.test_power);
				printf("pre_test_pwm_duty=               %d\r\n", acdc.pre_test_pwm_duty);
			}
		}

	}

	else if(!strncmp(ptr, "FBC0", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value22[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value22[i-1] = atoi(temp_ptr);
			i++;
			if(i == 3) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 3)
		{
			psfb.fb_test_hv = (unsigned short)temp_value22[0];
			psfb.fb_test_lv = (unsigned short)temp_value22[1];
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x42, 0x000, (char *)&psfb.fb_test_hv, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\nfb_test_hv=                      %d\r\n", psfb.fb_test_hv);
				printf("fb_test_lv=                      %d\r\n", psfb.fb_test_lv);
			}
		}

	}

	else if(!strncmp(ptr, "FBC1", 4))
	{

		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value23[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value23[i-1] = atoi(temp_ptr);
			i++;
			if(i == 4) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 4)
		{
			psfb.fb_buck_charging = (unsigned char)temp_value23[0];
			psfb.fb_buck_burst= (unsigned char)temp_value23[1];
			psfb.fb_buck_sr= (unsigned char)temp_value23[2];
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x43, 0x000, (char *)&psfb.fb_buck_charging, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\fb_buck_charging=                 %d\r\n", psfb.fb_buck_charging);
				printf("fb_buck_burst=                   %d\r\n", psfb.fb_buck_burst);
				printf("fb_buck_sr=                      %d\r\n", psfb.fb_buck_sr);
			}
		}

	}
	else if(!strncmp(ptr, "FBC2", 4))
	{
		int i = 0;
		char *temp_ptr = NULL;
		char *temp_ptr_1 = NULL;
		__IO unsigned short temp_value24[20] = {0};


		temp_ptr = strtok_r(ptr,",",&temp_ptr_1);
		while(temp_ptr != NULL)
		{
			if(i >= 1) temp_value24[i-1] = atoi(temp_ptr);
			i++;
			if(i == 2) break;
			temp_ptr = strtok_r(NULL, ",",&temp_ptr_1);
		}
		if(i == 2)
		{
			psfb.fb_discharging = (unsigned char)temp_value24[0];
			if(EN_QUE(CAN_ID_STD, CAN_RTR_DATA, 0x44, 0x000, (char *)&psfb.fb_discharging, 8) != HAL_OK) printf("ENQUE Fail\r\n");
			if(cprf)
			{
				printf("\r\nfb_discharging=                  %d\r\n", psfb.fb_discharging);
			}
		}
	}

	else printf("TCP Communication Protocol Not Found\r\n");
}

void tcp_server_tx(char *ptr)
{
	static char state = 0;
	static char tcp_tx_str[TCP_SERVER_BUFFER_SIZE];
	static int tcp_tx_str_len;

	switch(state)
	{
		case 0:
			memset(tcp_tx_str, 0, sizeof(tcp_tx_str));
			tcp_tx_str[0] = STX;
			sprintf(&tcp_tx_str[1],"BUCA,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"
					,buck_boost[0].SC_Voltage_1
					,buck_boost[0].SC_Voltage_2
					,buck_boost[0].SC_Voltage_3
					,buck_boost[0].SC_Voltage_4
					,buck_boost[0].SC_Current_1
					,buck_boost[0].SC_Current_2
					,buck_boost[0].SC_Current_3
					,buck_boost[0].SC_Current_4
					,buck_boost[0].Link_Voltage_1
					,buck_boost[0].Link_Voltage_2
					,buck_boost[0].Link_Voltage_3
					,buck_boost[0].Link_Voltage_4
					,buck_boost[0].Link_Current_1
					,buck_boost[0].Link_Current_2
					,buck_boost[0].Link_Current_3
					,buck_boost[0].Link_Current_4
					,buck_boost[0].Temp_1
					,buck_boost[0].Temp_2
					,buck_boost[0].Temp_3
					,buck_boost[0].Temp_4
					//,buck_boost[0].Firemware
					,buck_boost[0].SOC_1
					,buck_boost[0].SOC_2
					,buck_boost[0].SOC_3
					,buck_boost[0].SOC_4
					,buck_boost[0].Operation_State_1
					,buck_boost[0].Operation_State_2
					,buck_boost[0].Operation_State_3
					,buck_boost[0].Operation_State_4
					,buck_boost[0].Fault_State_1
					,buck_boost[0].Fault_State_2
					,buck_boost[0].Fault_State_3
					,buck_boost[0].Fault_State_4
					,buck_boost[0].Cycle_State_1
					,buck_boost[0].Cycle_State_2
					,buck_boost[0].Cycle_State_3
					,buck_boost[0].Cycle_State_4
					,buck_boost[0].Step_State_1
					,buck_boost[0].Step_State_2
					,buck_boost[0].Step_State_3
					,buck_boost[0].Step_State_4
					,buck_boost[0].Mode_ID_1
					,buck_boost[0].Mode_ID_2
					,buck_boost[0].Mode_ID_3
					,buck_boost[0].Mode_ID_4
					,buck_boost[0].Type_ID_1
					,buck_boost[0].Type_ID_2
					,buck_boost[0].Type_ID_3
					,buck_boost[0].Type_ID_4);
			tcp_tx_str_len = strlen(tcp_tx_str);
			tcp_tx_str[tcp_tx_str_len] = ETX;
			tcp_tx_str_len += 1;
			state++;
		break;
		case 1:
			if(tcp_write(tcp_active_pcbs, (const void *)tcp_tx_str, tcp_tx_str_len, TCP_SERVER_API_FLAGS) == ERR_OK)
			{
				tcp_output(tcp_active_pcbs);
				state++;
				//state=8;
			}
		break;
		case 2:
			memset(tcp_tx_str, 0, sizeof(tcp_tx_str));
			tcp_tx_str[0] = STX;
			sprintf(&tcp_tx_str[1],"BUCB,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"
					,buck_boost[1].SC_Voltage_1
					,buck_boost[1].SC_Voltage_2
					,buck_boost[1].SC_Voltage_3
					,buck_boost[1].SC_Voltage_4
					,buck_boost[1].SC_Current_1
					,buck_boost[1].SC_Current_2
					,buck_boost[1].SC_Current_3
					,buck_boost[1].SC_Current_4
					,buck_boost[1].Link_Voltage_1
					,buck_boost[1].Link_Voltage_2
					,buck_boost[1].Link_Voltage_3
					,buck_boost[1].Link_Voltage_4
					,buck_boost[1].Link_Current_1
					,buck_boost[1].Link_Current_2
					,buck_boost[1].Link_Current_3
					,buck_boost[1].Link_Current_4
					,buck_boost[1].Temp_1
					,buck_boost[1].Temp_2
					,buck_boost[1].Temp_3
					,buck_boost[1].Temp_4
					//,buck_boost[1].Firemware
					,buck_boost[1].SOC_1
					,buck_boost[1].SOC_2
					,buck_boost[1].SOC_3
					,buck_boost[1].SOC_4
					,buck_boost[1].Operation_State_1
					,buck_boost[1].Operation_State_2
					,buck_boost[1].Operation_State_3
					,buck_boost[1].Operation_State_4
					,buck_boost[1].Fault_State_1
					,buck_boost[1].Fault_State_2
					,buck_boost[1].Fault_State_3
					,buck_boost[1].Fault_State_4
					,buck_boost[1].Cycle_State_1
					,buck_boost[1].Cycle_State_2
					,buck_boost[1].Cycle_State_3
					,buck_boost[1].Cycle_State_4
					,buck_boost[1].Step_State_1
					,buck_boost[1].Step_State_2
					,buck_boost[1].Step_State_3
					,buck_boost[1].Step_State_4
					,buck_boost[1].Mode_ID_1
					,buck_boost[1].Mode_ID_2
					,buck_boost[1].Mode_ID_3
					,buck_boost[1].Mode_ID_4
					,buck_boost[1].Type_ID_1
					,buck_boost[1].Type_ID_2
					,buck_boost[1].Type_ID_3
					,buck_boost[1].Type_ID_4);
			tcp_tx_str_len = strlen(tcp_tx_str);
			tcp_tx_str[tcp_tx_str_len] = ETX;
			tcp_tx_str_len += 1;
			state++;
		break;
		case 3:
			if(tcp_write(tcp_active_pcbs, (const void *)tcp_tx_str, tcp_tx_str_len, TCP_SERVER_API_FLAGS) == ERR_OK)
			{
				tcp_output(tcp_active_pcbs);
				state++;
			}
		break;
		case 4:
			memset(tcp_tx_str, 0, sizeof(tcp_tx_str));
			tcp_tx_str[0] = STX;
			sprintf(&tcp_tx_str[1],"BUCC,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"
					,buck_boost[2].SC_Voltage_1
					,buck_boost[2].SC_Voltage_2
					,buck_boost[2].SC_Voltage_3
					,buck_boost[2].SC_Voltage_4
					,buck_boost[2].SC_Current_1
					,buck_boost[2].SC_Current_2
					,buck_boost[2].SC_Current_3
					,buck_boost[2].SC_Current_4
					,buck_boost[2].Link_Voltage_1
					,buck_boost[2].Link_Voltage_2
					,buck_boost[2].Link_Voltage_3
					,buck_boost[2].Link_Voltage_4
					,buck_boost[2].Link_Current_1
					,buck_boost[2].Link_Current_2
					,buck_boost[2].Link_Current_3
					,buck_boost[2].Link_Current_4
					,buck_boost[2].Temp_1
					,buck_boost[2].Temp_2
					,buck_boost[2].Temp_3
					,buck_boost[2].Temp_4
					//,buck_boost[2].Firemware
					,buck_boost[2].SOC_1
					,buck_boost[2].SOC_2
					,buck_boost[2].SOC_3
					,buck_boost[2].SOC_4
					,buck_boost[2].Operation_State_1
					,buck_boost[2].Operation_State_2
					,buck_boost[2].Operation_State_3
					,buck_boost[2].Operation_State_4
					,buck_boost[2].Fault_State_1
					,buck_boost[2].Fault_State_2
					,buck_boost[2].Fault_State_3
					,buck_boost[2].Fault_State_4
					,buck_boost[2].Cycle_State_1
					,buck_boost[2].Cycle_State_2
					,buck_boost[2].Cycle_State_3
					,buck_boost[2].Cycle_State_4
					,buck_boost[2].Step_State_1
					,buck_boost[2].Step_State_2
					,buck_boost[2].Step_State_3
					,buck_boost[2].Step_State_4
					,buck_boost[2].Mode_ID_1
					,buck_boost[2].Mode_ID_2
					,buck_boost[2].Mode_ID_3
					,buck_boost[2].Mode_ID_4
					,buck_boost[2].Type_ID_1
					,buck_boost[2].Type_ID_2
					,buck_boost[2].Type_ID_3
					,buck_boost[2].Type_ID_4);
			tcp_tx_str_len = strlen(tcp_tx_str);
			tcp_tx_str[tcp_tx_str_len] = ETX;
			tcp_tx_str_len += 1;
			state++;
		break;
		case 5:
			if(tcp_write(tcp_active_pcbs, (const void *)tcp_tx_str, tcp_tx_str_len, TCP_SERVER_API_FLAGS) == ERR_OK)
			{
				tcp_output(tcp_active_pcbs);
				state++;
			}
		break;
		case 6:
			memset(tcp_tx_str, 0, sizeof(tcp_tx_str));
			tcp_tx_str[0] = STX;
			sprintf(&tcp_tx_str[1],"BUCD,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"
					,buck_boost[3].SC_Voltage_1
					,buck_boost[3].SC_Voltage_2
					,buck_boost[3].SC_Voltage_3
					,buck_boost[3].SC_Voltage_4
					,buck_boost[3].SC_Current_1
					,buck_boost[3].SC_Current_2
					,buck_boost[3].SC_Current_3
					,buck_boost[3].SC_Current_4
					,buck_boost[3].Link_Voltage_1
					,buck_boost[3].Link_Voltage_2
					,buck_boost[3].Link_Voltage_3
					,buck_boost[3].Link_Voltage_4
					,buck_boost[3].Link_Current_1
					,buck_boost[3].Link_Current_2
					,buck_boost[3].Link_Current_3
					,buck_boost[3].Link_Current_4
					,buck_boost[3].Temp_1
					,buck_boost[3].Temp_2
					,buck_boost[3].Temp_3
					,buck_boost[3].Temp_4
					//,buck_boost[3].Firemware
					,buck_boost[3].SOC_1
					,buck_boost[3].SOC_2
					,buck_boost[3].SOC_3
					,buck_boost[3].SOC_4
					,buck_boost[3].Operation_State_1
					,buck_boost[3].Operation_State_2
					,buck_boost[3].Operation_State_3
					,buck_boost[3].Operation_State_4
					,buck_boost[3].Fault_State_1
					,buck_boost[3].Fault_State_2
					,buck_boost[3].Fault_State_3
					,buck_boost[3].Fault_State_4
					,buck_boost[3].Cycle_State_1
					,buck_boost[3].Cycle_State_2
					,buck_boost[3].Cycle_State_3
					,buck_boost[3].Cycle_State_4
					,buck_boost[3].Step_State_1
					,buck_boost[3].Step_State_2
					,buck_boost[3].Step_State_3
					,buck_boost[3].Step_State_4
					,buck_boost[3].Mode_ID_1
					,buck_boost[3].Mode_ID_2
					,buck_boost[3].Mode_ID_3
					,buck_boost[3].Mode_ID_4
					,buck_boost[3].Type_ID_1
					,buck_boost[3].Type_ID_2
					,buck_boost[3].Type_ID_3
					,buck_boost[3].Type_ID_4);
			tcp_tx_str_len = strlen(tcp_tx_str);
			tcp_tx_str[tcp_tx_str_len] = ETX;
			tcp_tx_str_len += 1;
			state++;
		break;
		case 7:
			if(tcp_write(tcp_active_pcbs, (const void *)tcp_tx_str, tcp_tx_str_len, TCP_SERVER_API_FLAGS) == ERR_OK)
			{
				tcp_output(tcp_active_pcbs);
				state++;
			}
		break;
		case 8:
				memset(tcp_tx_str, 0, sizeof(tcp_tx_str));
				tcp_tx_str[0] = STX;
				sprintf(&tcp_tx_str[1],"ACDC,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d"
						,acdc.ac_r_voltage
						,acdc.ac_s_voltage
						,acdc.ac_t_voltage
						,acdc.ac_r_current
						,acdc.ac_s_current
						,acdc.ac_t_current
						,acdc.ac_frequency_r
						,acdc.dc_voltage_r
						,acdc.dc_current
						,acdc.temperature
						,acdc.apparent_power
						,acdc.active_power
						,acdc.reactive_power
						,acdc.power_factor
						,acdc.dc_power
						,acdc.flag1
						,acdc.flag2
						,acdc.flag3
						,acdc.flag4
						);
				tcp_tx_str_len = strlen(tcp_tx_str);
				tcp_tx_str[tcp_tx_str_len] = ETX;
				tcp_tx_str_len += 1;
				state++;



		break;
		case 9:
			if(tcp_write(tcp_active_pcbs, (const void *)tcp_tx_str, tcp_tx_str_len, TCP_SERVER_API_FLAGS) == ERR_OK)
			{
				tcp_output(tcp_active_pcbs);
				state++;
			}
		break;
		case 10:
				memset(tcp_tx_str, 0, sizeof(tcp_tx_str));
				tcp_tx_str[0] = STX;
				sprintf(&tcp_tx_str[1],"FBDA,%d,%d,%d,%d,%d,%d,%d,%d"
						,psfb.fb_lv_v
						,psfb.fb_lv_i
						,psfb.fb_hv_v
						,psfb.fb_hv_i
						,psfb.fb_fault_1
						,psfb.fb_fault_2
						,psfb.fb_fault_3
						,psfb.fb_fault_4
						);
				tcp_tx_str_len = strlen(tcp_tx_str);
				tcp_tx_str[tcp_tx_str_len] = ETX;
				tcp_tx_str_len += 1;
				state++;
		break;
		case 11:
			if(tcp_write(tcp_active_pcbs, (const void *)tcp_tx_str, tcp_tx_str_len, TCP_SERVER_API_FLAGS) == ERR_OK)
			{
				tcp_output(tcp_active_pcbs);
				state = 0;
			}
		break;
		default:
		break;
	}
}
#endif /* LWIP_TCP */
#endif
