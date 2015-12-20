#include "Physical_layer.h"

#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

#define PORT 8465
#define MAX_BUF_LEN 160





int num_of_frames_sent;
int num_of_frames_retr;
int num_of_ack_sent;
int num_of_ack_received;
int num_of_dup_frames;
int num_of_bytes_sent;
//states factors
int sockfd;
basedata b;



void states_reset()
{
	num_of_frames_sent=0;
	num_of_frames_retr=0;
	num_of_ack_sent=0;
	num_of_ack_received=0;
	num_of_bytes_sent=0;
	num_of_dup_frames=0;
}

int random_event()
{
	int r= rand()%100;
	return r;
}

void phy_send(frame f)
{
	if(random_event()<=b.drop_rate)//data drop
	{
		printf("Frame has been dropped!\n");
		return;
	}

	unsigned char * contex = (unsigned char *)&f;

	if(random_event()<=b.corrupt_rate)//data corrupt
	{
		contex[0]= 0;
		printf("Frame data corrupted!\n");
	}

	frame *new_fr=(frame*)contex;
	frame fr=*new_fr;
	//stats factors increase
	num_of_frames_sent++;
	num_of_bytes_sent+=124;
	printf("DEBUG: %s, %s\n", fr.msg.cmd, fr.msg.data);
	if(send(sockfd,contex,MAX_BUF_LEN,0)==-1)
	{
		perror("send");
	}

}

void phy_recv()
{
	int buffer_lenth;
	unsigned char *buf= (unsigned char *)malloc(sizeof(unsigned char)*MAX_BUF_LEN);

	buffer_lenth=recv(sockfd,buf,MAX_BUF_LEN,0);
	if(buffer_lenth>0)
	{
		frame *newframe=(frame*)buf;
		frame fra=*newframe;
		datalink_recv(fra);
	}

}

void give_report()
{
	printf("Drop rate is :%d %%\n",b.drop_rate);
	printf("corrupt rate is :%d %%\n",b.corrupt_rate);
	printf("Total number of frames sent is: %d\n",num_of_frames_sent);
	printf("Total Data sent : %d bytes.\n",num_of_bytes_sent);
	
}

void start_server()
{
	int server_socket, addrlen;
    int max_sd;
    int client_activity;
    int msg_bytes;
    

    struct sockaddr_in address;

    

    fd_set readfds;
    //create the server_socket for the server
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(1);
    }

    //set what type of socket we created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    //bind the socket to our port 8888
    if (bind(server_socket, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(1);
    }


    //specify the max number of pending connections
    if (listen(server_socket, 10) < 0) {
        perror("listen");
        exit(1);
    }

    //accept incoming connection
    addrlen = sizeof(address);
    

    //create thread point to handle server side input
    while(true)
    {

        //set socket set to zero
        FD_ZERO(&readfds);

        //add our server socket to the set
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;
        if(sockfd > 0){
            FD_SET(sockfd, &readfds);
            max_sd = sockfd;
        }

        //wait for client activity our timeout is null wait forever until we get something
        client_activity = select( max_sd + 1, &readfds, NULL, NULL, NULL);

        //if client_activity is less than -1 then we had a selection error on creation
        if((client_activity < 0))
        {
            printf("select error\n");
        }


        // if first client_activity from new socket we have new connection
        if(FD_ISSET(server_socket, &readfds))
        {
            if(( sockfd = accept(server_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
            {
                perror("accept");
                exit(1);
            }

            

        }
        else if(FD_ISSET(sockfd, &readfds))
        {

            phy_recv();
        }
    }
}

void start_client()
{
	struct hostent *he;
    struct sockaddr_in their_addr;
    char input[MAX_BUF_LEN];
    char text[MAX_BUF_LEN];
    int msg_bytes, readval, max_sd, activity;


    fd_set readfds;
    fd_set writefds;

    he = gethostbyname("localhost");
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        perror("socket");
        exit(1);
    }
    // Set what type of socket we are connecting to
    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(PORT);
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(&(their_addr.sin_zero), '\0', 8);

    if(connect(sockfd, (struct sockaddr*)&their_addr, sizeof(struct sockaddr)) == -1){
        perror("connect");
        exit(1);
    }


    char buffer[MAX_BUF_LEN];
    //set up the buffer
    memset(buffer, '\0', sizeof(buffer));

    while(1) {
         //set socket set to zero
         FD_ZERO(&readfds);

         //add our server socket to the set
         FD_SET(sockfd, &readfds);
         max_sd = sockfd;



         //wait for activity our timeout is null wait forever until we get something
         activity = select(max_sd + 1, &readfds, &writefds, NULL, NULL);

         //if activity is less than -1 then we had a selection error on creation
         if (activity == -1) {
             perror("select");
         }

         if (FD_ISSET(sockfd, &readfds)) {
             phy_recv();
         }

    }

}

void *init_program(void * args)
{
	b= *(basedata *)args;
	srand(time(0));
	if(b.is_server==1){
		start_server();
	}else
	{
		start_client();
	}

}