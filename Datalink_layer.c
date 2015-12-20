#include "Datalink_layer.h"
#include "Physical_layer.h"
#include "Application_layer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define MAX_FRAME_NUMBER 20000
#define WINDOW_SIZE 20

int base=1;
int nextseq=1;
int X = WINDOW_SIZE;
int rcv_base = 1;
int t;
frame sender_buffer[MAX_FRAME_NUMBER];
frame recv_buffer[MAX_FRAME_NUMBER];
frame ack_buffer[MAX_FRAME_NUMBER];
int expect_seq = 1;
frame send_frame;
int timer_active = 0;
int timeout = 3;
time_t current_time;
time_t start_time;
pthread_t time_out_for_GBN;
pthread_t time_out_for_single_SR;
pthread_t time_out_for_SR[40];

unsigned int checksum(frame f)
{
	unsigned s=0;
	unsigned char *tem=(unsigned char*)&f;
	int a;
	for(a=0;a<10;a+=2)
	{
		unsigned short x = *(unsigned short*)&tem[a];
		s+=x;
	}
	s= (s & 0xFFFF)+(s>>16);
	s+=(s>>16);
	return ~s;
}

int check_checksum(frame f)
{
	if(checksum(f)>0) return 1;
	return 0;
}

int is_expected(int seq_num)
{
	printf("exp:%d,seq:%d\n",expect_seq,seq_num );
	if(seq_num==expect_seq)return 1;
	else return 0;
}

frame ack_frame(int expect_seq,int ack,int is_GBN)
{
	frame ackframe;
	ackframe.ack_num=expect_seq;
	ackframe.ack = 1;
	ackframe.checksum=0;
	ackframe.is_GBN=is_GBN;
	ackframe.seq_num=expect_seq;
	return ackframe;
}

frame make_a_frame(message m, int newseq,int is_GBN)
{
	frame f;
	f.seq_num = newseq;
	f.ack_num=1;
	f.ack=0;
	f.checksum=0;
	f.msg=m;
	f.is_GBN=is_GBN;
	return f;
}
void GBN_start_timer()
{
	if(timer_active==1){
		start_time=time(0);
	}else
	{
		pthread_create(&time_out_for_GBN,NULL,GBN_timer,NULL);
		timer_active=1;
	}
}

void GBN_stop_timer()
{
	timer_active=0;
	pthread_cancel(time_out_for_GBN);
}

void SR_start_timer(int i)
{
	t=i;
	printf("Timer for frame %d starts!\n",t);
	pthread_create(&time_out_for_single_SR,NULL,SR_timer,t);
	time_out_for_SR[t]=time_out_for_single_SR;
}
void SR_stop_timer(int i)
{
	pthread_cancel(time_out_for_SR[i]);
}

void *SR_timer(int i)
{
	int s;
	int index = i;
	if((s=pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL)) != 0){
        perror("pthread_setcanceltype");
    }
    time_t sr_start_time;
    sr_start_time  = time(0);
    current_time=time(0);
    while (timeout > current_time - sr_start_time) {
        sleep(1);
        current_time = time(0);
     
    }
    timer_active = 0;
    printf("Timeout!\n");
    SR_retransmission(index);
}

void *GBN_timer()
{
	int a;

	if((a=pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL))!=0)
	{
		perror("pthread_setcanceltype");
	}
	start_time = time(0);
	current_time = time(0);
	while(timeout>current_time-start_time)
	{
		sleep(1);
		current_time=time(0);
	}
	timer_active=0;
	printf("Timeout!\n");
	GBN_retransmission();
	
}
void GBN_retransmission()
{
	int t;
	GBN_start_timer();
	for(t=base;t<nextseq;t++)
	{
		printf("Resending Frame %d\n",t-1);
		phy_send(sender_buffer[t-1]);
	}
}

void GBN_sender_send(message m)
{
	if(nextseq<(base+X))
	{
		sender_buffer[nextseq-1]=make_a_frame(m,nextseq,1);
		int check=checksum(sender_buffer[nextseq-1]);
		sender_buffer[nextseq-1].checksum=check;
		phy_send(sender_buffer[nextseq-1]);
		if(base==nextseq) GBN_start_timer();
		nextseq++;
	}
	else
	{
		sleep(1);
		GBN_sender_send(m);
	}

}

void GBN_sender_recv(frame f)
{
	if(check_checksum(f)==1)
	{
		if(f.seq_num>MAX_FRAME_NUMBER){
			nextseq=1;
		}
		base = f.seq_num+1;
		if(base>MAX_FRAME_NUMBER)
		{
			base=1;
		}
		printf("Window Information: Base_number: %d,Seq_number: %d, Ack_number: %d, Expecting_seq_number: %d\n",base,f.seq_num,f.ack_num,nextseq );
		if(base==nextseq) GBN_stop_timer();
		else GBN_start_timer();
	}else
	{
		printf("Frame %d corrupted!\n",f.seq_num);
	}
}

void GBN_receiver_recv(frame f)
{
	message m;
	if(check_checksum(f)==1)
	{
		if(is_expected(f.seq_num))
		{
			m=f.msg;
			handle_message(m,1);
			send_frame = ack_frame(f.seq_num,1,1);

			int check=checksum(send_frame);
			send_frame.checksum=check;

			phy_send(send_frame);
			expect_seq++;
		}
		else
		{
			printf("Not receiving expected frame!\n");
		}
	}
	else
	{
		printf("Frame %d corrupted!\n", f.seq_num);
	}
}

void SR_retransmission(int i)
{
	SR_start_timer(i);
	printf("Resending frame: %d\n",i );
	phy_send(sender_buffer[i]);
}

void SR_sender_send(message m)
{
	if(nextseq<(base+X))
	{
		sender_buffer[nextseq-1]=make_a_frame(m,nextseq,0);
		int check=checksum(sender_buffer[nextseq-1]);
		sender_buffer[nextseq-1].checksum=check;
		phy_send(sender_buffer[nextseq-1]);
		SR_start_timer(nextseq-1);
		nextseq++;
	}
	else{
		sleep(1);
		SR_sender_send(m);
	}
}

void SR_sender_recv(frame f)
{
	if(check_checksum(f)==1)
	{
		if(f.seq_num>MAX_FRAME_NUMBER)
		{
			nextseq=1;
		}
		base = f.seq_num+1;
		if(base>MAX_FRAME_NUMBER)
		{
			base = 1;
		}
		printf("Window Information: Base_number: %d,Seq_number: %d, Ack_number: %d, Expecting_seq_number: %d\n",base,f.seq_num,f.ack_num,nextseq );
		SR_stop_timer(f.ack_num-1);
	}
	else{
		printf("Frame %d corrupted!\n",f.seq_num);
	}
}

void SR_receiver_recv(frame f)
{
	message m;
	if(check_checksum(f)==1)
	{
		if(is_expected(f.seq_num))
		{
			m=f.msg;
			handle_message(m,0);
			ack_buffer[f.seq_num-1]=ack_frame(f.seq_num,1,0);
			int check=checksum(ack_buffer[f.seq_num-1]);
			ack_buffer[f.seq_num-1].checksum=check;

			phy_send(ack_buffer[f.seq_num-1]);
			expect_seq++;
			rcv_base++;

			if(is_expected(recv_buffer[expect_seq-1].seq_num))
			{
				SR_receiver_recv(recv_buffer[expect_seq-1]);
				recv_buffer[expect_seq-1].seq_num=0;
			}
		}
		else
		{
			if((f.seq_num<(rcv_base+X))&&(f.seq_num>rcv_base))
			{
				recv_buffer[f.seq_num-1]=f;
			}else{
				phy_send(ack_buffer[f.seq_num-1]);
			}
		}
	}else
	{
		printf("Frame %d corrupted!\n", f.seq_num);
	}
}
int datalink_send(message m,int is_GBN)
{
    int stat=1;
    if(is_GBN==1)
    {
    	GBN_sender_send(m);
    }else if(is_GBN==0)
    {
    	SR_sender_send(m);
    }else
    {
    	stat=0;
    }
    return stat;
}

void datalink_recv(frame f)
{
   if(f.is_GBN==1)
   {
   		if(f.ack==0)
	   	  GBN_receiver_recv(f);
	   	else if(f.ack==1)
	   	{
	   		GBN_sender_recv(f);
	   		printf("GBN_sender buffer is receiving a new frame!\n");
	   	}
   }else if(f.is_GBN==0)
   {
   		if(f.ack==0)
   		{
   			SR_receiver_recv(f);
   		}else if(f.ack==1)
   		{
   			SR_sender_recv(f);
   		}
   }

}