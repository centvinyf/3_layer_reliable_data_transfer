#ifndef DATALINK_LAYER_H
#define DATALINK_LAYER_H
#include "Application_layer.h"

typedef struct frame
{
	unsigned int seq_num;
	unsigned int ack_num;
	unsigned int ack:1;
	unsigned int checksum:16;
	int is_GBN;
	message msg;
}frame;

unsigned int checksum(frame f);


int check_checksum(frame f);


int is_expected(int seq_num);


frame ack_frame(int expect_seq,int ack,int is_GBN);

frame make_a_frame(message m, int newseq,int is_GBN);

void GBN_start_timer();

void GBN_stop_timer();

void GBN_retransmission();

void *GBN_timer();

void GBN_sender_send(message m);

void GBN_sender_recv(frame f);


void GBN_receiver_recv(frame f);

void SR_start_timer(int i);

void SR_stop_timer(int i);

void *SR_timer(int i);

int datalink_send(message m,int is_GBN);

void SR_retransmission(int i);


void SR_sender_send(message m);


void SR_sender_recv(frame f);

void SR_receiver_recv(frame f);
void datalink_recv(frame f);

#endif