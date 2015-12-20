#include "Physical_layer.h"
#include "Datalink_layer.h"
#include "Application_layer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


int main(int argc, char *argv[])
{
	if(argc !=5)
	{
		printf("You should enter the right format to start the program\n");
		printf("A B C D\nA=1 for server/ A=0 for client\nB= drop rate\nC= corrupt rate\nD=1 for GBN/ D=0 for SR\n");
		exit(1);
	}
	message m;
	frame f;
	basedata bb;

	bb.is_server = atoi(argv[1]);
	bb.drop_rate = atoi(argv[2]);
	bb.corrupt_rate = atoi(argv[3]);
	bb.is_GBN = atoi(argv[4]);

	pthread_t Physical_layer_thread;
	pthread_create(&Physical_layer_thread,NULL,init_program,&bb);
	while(1)
	{
		app(m,atoi(argv[4]));
	}
}