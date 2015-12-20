#ifndef PHYSICAL_LAYER_H
#define PHYSICAL_LAYER_H
#include "Datalink_layer.h"
typedef struct basedata{

int is_server;
int drop_rate;
int corrupt_rate;
int is_GBN;
//basic factors
}basedata;

void states_reset();


int random_event();


void phy_send(frame f);


void phy_recv();


void give_report();


void start_server();


void start_client();


void *init_program(void * args);

#endif