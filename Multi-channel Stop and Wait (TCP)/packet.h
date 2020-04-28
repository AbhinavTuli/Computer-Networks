#include <stdio.h> 
#include <string.h> //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> //close 
#include <arpa/inet.h> //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <fcntl.h>
#include <poll.h>
#include<stdbool.h>

#define PACKET_SIZE 100
#define TMOUT 2000 //in milliseconds
#define PDR 10 //packet drop rate in percentage

#define TRUE 1 
#define FALSE 0 
#define PORT 8888 
struct packet{
    char payload[PACKET_SIZE-22];
    int size;
    int sqNo;
    int isLast; //1 if last, 0 otherwise
    int isData; //1 if data, 0 otherwise
    int chId; // 0 for ch0, 1 for ch1
};