/* A simple SocketCAN example */

#include "../include/global.h"
#include "../include/can-dev.h"


int soc;
int read_can_port;
int thr_can_id;
pthread_t can_recvThread = 0;
int thread_can_end = 0;

void *can_recv_thread(void *data) 
{
    struct can_frame frame_rd;
    int recvbytes = 0;

    read_can_port = 1;
    while(read_can_port)
    {
        struct timeval timeout = {1, 0};
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(soc, &readSet);

        if (select((soc + 1), &readSet, NULL, NULL, &timeout) >= 0)
        {
            if (!read_can_port)
            {
                break;
            }
            if (FD_ISSET(soc, &readSet))
            {
                recvbytes = read(soc, &frame_rd, sizeof(struct can_frame));
                if(recvbytes)
                {
                    printf("dlc = %d, data = %s\n", frame_rd.can_dlc,frame_rd.data);

                }
            }
        }

    }

}

int can_open_port(const char *port)
{
	struct ifreq ifr;
	struct sockaddr_can addr;

	/* open socket */
	soc = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if(soc < 0)
	{
		LOGE("can socket error\n");
		return (-1);
	}

	addr.can_family = AF_CAN;
	strcpy(ifr.ifr_name, port);

	if (ioctl(soc, SIOCGIFINDEX, &ifr) < 0)
	{
		LOGE("can ioctl error\n");
		return (-1);
	}

	addr.can_ifindex = ifr.ifr_ifindex;

	fcntl(soc, F_SETFL, O_NONBLOCK);

	if (bind(soc, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		LOGE("can bind error\n");
		return (-1);
	}

	thread_can_end=0;
	thr_can_id = pthread_create(&can_recvThread, NULL, can_recv_thread, NULL);
	if(thr_can_id < 0)
	{
		LOGE("can_recvThread create error\n");
	}
	
	return 0;
}

int can_send_port(struct can_frame *frame)
{
	int retval;
	if(soc < 0) printf("can socket error\n");
	retval = write(soc, frame, sizeof(struct can_frame));
	printf("Wrote %d bytes\n", retval);
	if (retval != sizeof(struct can_frame))
	{
		return (-1);
	}
	else
	{
		return (0);
	}
}



int can_close_port()
{

	thread_can_end=1;
	if(can_recvThread!=0)
	{
		pthread_join(can_recvThread,NULL);
		can_recvThread=0;
	}

	close(soc);	
	
	return 0;
}

/*
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <linux/can.h>
#include <linux/can/raw.h>

int
main(void)
{
	int s;
	int nbytes;
	struct sockaddr_can addr;
	struct can_frame frame;
	struct ifreq ifr;

	const char *ifname = "vcan0";

	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) == -1) {
		perror("Error while opening socket");
		return -1;
	}

	strcpy(ifr.ifr_name, ifname);
	ioctl(s, SIOCGIFINDEX, &ifr);
	
	addr.can_family  = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	printf("%s at index %d\n", ifname, ifr.ifr_ifindex);

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		perror("Error in socket bind");
		return -2;
	}

	frame.can_id  = 0x123;
	frame.can_dlc = 2;
	frame.data[0] = 0x11;
	frame.data[1] = 0x22;

	nbytes = write(s, &frame, sizeof(struct can_frame));

	printf("Wrote %d bytes\n", nbytes);
	
	return 0;
}
*/
