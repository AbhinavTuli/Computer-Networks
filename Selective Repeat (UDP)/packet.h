#include<stdbool.h>
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
#define PACKET_SIZE 100
#define PDR 10 //packet drop rate in percentage

#define TRUE 1 
#define FALSE 0 

#define PORT0 6666 //relay0
#define PORT1 7777 //relay1
#define PORT2 8867 //server
#define PORT3 9009 //client


#pragma pack(1)
struct packet{
    char payload[PACKET_SIZE-10];
    int size;
    int sqNo;
    bool isLast; //1 if last, 0 otherwise
    bool isData; //1 if data, 0 otherwise
};

struct windowNode{
    struct packet pkt;
    int isAcked; //0 if haven't reveived ack, 1 if ack received
    struct windowNode* next;
};

char* get_current_time(){
    struct timeval  tv;
    gettimeofday(&tv, NULL);

    long long time_in_micro = (tv.tv_sec) * 1000000 + (tv.tv_usec) ;
    char str[100];
    sprintf(str,"%lld",time_in_micro);
    return str;
}