#include "Application_layer.h"
#include "Datalink_layer.h"
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <dirent.h>
#include <time.h>
#include <unistd.h>

char file_name[95];

time_t start_time;
time_t end_time;

void show_command()
{
	printf("You can use those commands:\n");
	printf("SEND : to send a file from client to server\n");
	printf("LIST : to show what's in server dir\n");
	
}

void send_finish_msg(int is_GBN)
{
	message response;
	strcpy(response.cmd,"DONE");
	strcpy(response.data,"FINISHED!!!");
	response.cmd[strlen(response.cmd)]='\0';
	response.data[strlen(response.data)]='\0';
	datalink_send(response,is_GBN);
}

void handle_message(message m,int is_GBN)
{
	message response;
	if(!strncasecmp(m.cmd,"LIST",4))
	{
		DIR *path;
		struct dirent *p;
		path = opendir(".");
		if(path!=NULL){
			while((p=readdir(path))!=NULL)
			{
				strcpy(response.cmd,"RECV");
				strcpy(response.data,p->d_name);
				response.cmd[strlen(response.cmd)]='\0';
				response.data[strlen(response.data)]='\0';
				datalink_send(response,is_GBN);
			}
			(void)closedir(path);
		}else
		{
			strcpy(response.cmd,"RECV");
				strcpy(response.data,"There is no file in server\n");
				response.cmd[strlen(response.cmd)]='\0';
				response.data[strlen(response.data)]='\0';
				datalink_send(response,is_GBN);
		}
		send_finish_msg(is_GBN);
	}
	else if(!strncasecmp(m.cmd,"RECV",4))
	{
		printf("The context of this message is %s\n",m.data);
	}
	else if(!strncasecmp(m.cmd,"SEND",4))
	{
		if(!strcmp(m.data,"FINISH"))
		{
			send_finish_msg(is_GBN);
			printf("File received Successfully!");// send_finish_msg(is_GBN);
		}
		else{

		FILE *fp;
		fp= fopen(file_name,"a");
		if(fp!=NULL)
		{
			fprintf(fp, "%s",m.data);
		}else{
			printf("File cannot be open\n");
		}
		fclose(fp);
		
	}

	}
	else if(!strncasecmp(m.cmd,"NFN",3))
	{
		strcpy(file_name,m.data);
		file_name[strlen(file_name)]='\0';
	}
	else if (!strncasecmp(m.cmd,"DONE",4))
	{
		printf("File received Successfully!\n");
	}
}



void send_message(message m, int is_GBN)
{
	start_time=time(0);
	if(!strncasecmp(m.cmd,"SEND",4))
	{
		FILE *fp;
		char temp[94];
		char *buff=(char *)malloc(sizeof(char)*94);
		char* origin_file_name=(char *)malloc(sizeof(char)*94);
		char* new_file_name=(char *)malloc(sizeof(char)*94);
		printf("Please enter the file name: \n");
		fgets(buff,94,stdin);
		strcpy(origin_file_name,buff);
		origin_file_name=strtok(origin_file_name," \n");
		printf("Please enter the file name you want to save as: \n");
		fgets(buff,94,stdin);
		strcpy(new_file_name,buff);
		new_file_name=strtok(new_file_name," \n");

		strcpy(m.data,new_file_name);
		strcpy(m.cmd,"NFN");
		datalink_send(m,is_GBN);

		fp = fopen(origin_file_name,"r");
		if(fp==NULL)
		{
			printf("File does not exist, please double check！\n");
		}
		else{
			while(fgets(buff,sizeof(buff),fp)!=NULL)
			{
				strcpy(m.cmd,"SEND");
				strcpy(m.data,buff);
				datalink_send(m,is_GBN);
			}
			fclose(fp);
		}
		// strcpy(m.cmd,"SEND");
		strcpy(m.data,"FINISH");
		datalink_send(m,is_GBN);


	}
	else if(!strncasecmp(m.cmd,"LIST",4))
	{
		strcpy(m.data,"Listing the files!");
		datalink_send(m,is_GBN);
	}
	else
	{
		printf("Your command is not valid! Please enter a new one!\n");
		printf("Please enter a command:\n");
		char *tem=(char *)malloc(sizeof(char)*94);
		fgets(tem,sizeof(tem),stdin);
		tem=strtok(tem," \n");
		strcpy(m.cmd,tem);
		send_message(m,is_GBN);
	}
}

void app(message m, int is_GBN)
{
	show_command();
	printf("Please enter a valid command:\n");
	char *tem=(char *)malloc(sizeof(char)*94);
		fgets(tem,sizeof(tem),stdin);
		tem=strtok(tem," \n");
		strcpy(m.cmd,tem);
		send_message(m,is_GBN);
}