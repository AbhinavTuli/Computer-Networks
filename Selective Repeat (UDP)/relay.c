#include "packet.h"

int main(){
    struct sockaddr_in relay1_addr,relay2_addr, client_addr,server_addr,temp_addr;
    int len=sizeof(relay1_addr);

    relay1_addr.sin_family = AF_INET; 
	relay1_addr.sin_port = htons(PORT0); 

    relay2_addr.sin_family = AF_INET; 
	relay2_addr.sin_port = htons(PORT1); 

    server_addr.sin_family = AF_INET; 
	server_addr.sin_port = htons(PORT2); 

    client_addr.sin_family = AF_INET; 
	client_addr.sin_port = htons(PORT3); 

    int relay1,relay2; //sockets

    if ((relay1 = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{ 
		printf("\n Error while creating socket! \n"); 
		return -1; 
	} 
    if ((relay2 = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{ 
		printf("\n Error while creating socket! \n"); 
		return -1; 
	} 

    if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr)<=0) { 
		printf("\nAddress is Invalid!\n"); 
		return -1; 
	} 

	// Converting address to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &relay1_addr.sin_addr)<=0) { 
		printf("\nAddress is Invalid!\n"); 
		return -1; 
	} 
    if(inet_pton(AF_INET, "127.0.0.1", &relay2_addr.sin_addr)<=0) { 
		printf("\nAddress is Invalid!\n"); 
		return -1; 
	} 

    if(inet_pton(AF_INET, "127.0.0.1", &client_addr.sin_addr)<=0) 
	{ 
		printf("\nAddress is Invalid!\n"); 
		return -1; 
	} 

    if( bind(relay1 , (struct sockaddr*)&relay1_addr, len ) == -1) {
        printf("\nError while binding!\n"); 
		return -1; 
    }

    if( bind(relay2 , (struct sockaddr*)&relay2_addr, len ) == -1) {
        printf("\nError while binding!\n"); 
		return -1; 
    }

    struct pollfd poll1[1];
    struct pollfd poll2[1];
    poll1[0].fd=relay1;
    poll1[0].events=POLLIN;
    poll2[0].fd=relay2;
    poll2[0].events=POLLIN;
    int num_events1,num_events2;
    num_events1=num_events2=0;
    bool isLastAcked=false;
    while(1){ 
        if(num_events1>0){
            int flg=(rand()%100)+1;
            int delay=(rand()%2000); //calculating delay in microseconds
            struct packet p;
            recvfrom(relay1,&p,PACKET_SIZE,0,(struct sockaddr *) &temp_addr, &len);
            if(p.isData==1){ //coming from client
                if(flg>PDR){
                    usleep(delay);
                    printf("RELAY1 R %s DATA %d CLIENT RELAY1\n",get_current_time(),p.sqNo);
                    sendto(relay1, &p, 100 , 0 , (struct sockaddr *) &server_addr,len );
                    printf("RELAY1 S %s DATA %d RELAY1 SERVER\n",get_current_time(),p.sqNo);
                }
            }
            else{ //coming from server, i.e ACK
                printf("RELAY1 R %s ACK %d SERVER RELAY1\n",get_current_time(),p.sqNo);
                sendto(relay1, &p, 100 , 0 , (struct sockaddr *) &client_addr,len );
                printf("RELAY1 S %s ACK %d RELAY1 CLIENT\n",get_current_time(),p.sqNo);
                if(p.isLast){
                    isLastAcked=true;
                }
            }
            //num_events1--;
        }
        if(num_events2>0){
            int flg=(rand()%100)+1;
            int delay=(rand()%2000); //calculating delay in microseconds
            struct packet p;
            recvfrom(relay2,&p,PACKET_SIZE,0,(struct sockaddr *) &temp_addr, &len);
            if(p.isData==1){
                if(flg>PDR){
                    usleep(delay);
                    printf("RELAY2 R %s DATA %d CLIENT RELAY2\n",get_current_time(),p.sqNo);
                    sendto(relay2, &p, 100 , 0 , (struct sockaddr *) &server_addr,len );
                    printf("RELAY2 S %s DATA %d RELAY2 SERVER\n",get_current_time(),p.sqNo);
                }
            }
            else{ //coming from server, i.e ACK
                sendto(relay2, &p, 100 , 0 , (struct sockaddr *) &client_addr,len );
                if(p.isLast){
                    isLastAcked=true;
                }
            }
        }
        num_events1 = poll(poll1, 1, 0);
        num_events2 = poll(poll2, 1, 0);
    }


}