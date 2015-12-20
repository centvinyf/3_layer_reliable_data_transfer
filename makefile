all: project2

project2: main.o Application_layer.o Datalink_layer.o Physical_layer.o
	gcc -g main.o Application_layer.o Datalink_layer.o Physical_layer.o -o project2 -lpthread

main.o: main.c
	gcc -g -c main.c -lpthread

Application_layer.o: Application_layer.c Application_layer.h
	gcc -g -c Application_layer.c -lpthread

Datalink_layer.o: Datalink_layer.c Datalink_layer.h
	gcc -g -c Datalink_layer.c -lpthread

Physical_layer.o: Physical_layer.c Physical_layer.h
	gcc -g -c Physical_layer.c -lpthread
