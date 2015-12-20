#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H
typedef struct message
{
	char cmd[5];
	char data[95];
}message;


void show_command();

void send_finish_msg(int is_GBN);


void handle_message(message m,int is_GBN);


void send_message(message m, int is_GBN);

void app(message m, int is_GBN);

#endif

