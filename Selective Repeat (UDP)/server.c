#include "packet.h"
int main(){
    FILE*ptr=fopen("output.txt","w"); 
    struct sockaddr_in server_addr,incoming_addr,relay1_addr;
    int len=sizeof(incoming_addr);

    server_addr.sin_family = AF_INET; 
	server_addr.sin_port = htons(PORT2); 

    relay1_addr.sin_family = AF_INET; 
	relay1_addr.sin_port = htons(PORT0); 
    int server; //sockets

    if ((server = socket(AF_INET, SOCK_DGRAM, 0)) < 0) 
	{ 
		printf("\n Error while creating socket! \n"); 
		return -1; 
	} 

	// Converting address to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr)<=0) { 
		printf("\nAddress is Invalid!\n"); 
		return -1; 
	} 

    if(inet_pton(AF_INET, "127.0.0.1", &relay1_addr.sin_addr)<=0) { 
		printf("\nAddress is Invalid!\n"); 
		return -1; 
	} 

    if( bind(server , (struct sockaddr*)&server_addr, len ) == -1) {
        printf("\nError while binding!\n"); 
		return -1; 
    }

    struct pollfd server_poll[1];
    server_poll[0].fd=server;
    server_poll[0].events=POLLIN;
    int num_events;
    num_events=0;
    bool isLastAcked=false;
    char buffer[(PACKET_SIZE-10)*8];
    char buffer2[(PACKET_SIZE-10)*8];
    int bufferct=0;
    int buffer2ct=0;
    int lstBufferCt=-1;
    while(1){ 
        if(isLastAcked && lstBufferCt==bufferct){
            buffer[bufferct*(PACKET_SIZE-10)]='\0';
            fflush(ptr);
            fprintf(ptr,"%s",buffer);
            break;
        }
        if(bufferct==8){
            fflush(ptr);
            fprintf(ptr,"%s",buffer);
            bufferct=buffer2ct;
            buffer2ct=0;
            memcpy(buffer,buffer2,(PACKET_SIZE-10)*8);
            fseek(ptr, -1, SEEK_CUR);
        }
        if(num_events>0){
            struct packet p;
            int k=recvfrom(server,&p,PACKET_SIZE,0,(struct sockaddr *) &incoming_addr, &len);
            if (k!=-1){
                if(p.sqNo%2==0){
                    printf("SERVER R %s DATA %d RELAY2 SERVER\n",get_current_time(),p.sqNo);
                }
                else{
                    printf("SERVER R %s DATA %d RELAY1 SERVER\n",get_current_time(),p.sqNo);
                }
                if(bufferct>5 && p.sqNo<4){
                    for(int j=0;j<(PACKET_SIZE-10);j++){
                        buffer2[(p.sqNo)*(PACKET_SIZE-10)+j]=p.payload[j];
                    }
                        buffer2ct++;
                }
                else{
                    for(int j=0;j<(PACKET_SIZE-10);j++){
                        buffer[(p.sqNo)*(PACKET_SIZE-10)+j]=p.payload[j];
                    }
                    bufferct++;
                }
                struct packet ack;
                ack.sqNo=p.sqNo;
                ack.isData=0;
                ack.isLast=p.isLast;
                if(ack.isLast){
                    isLastAcked=1;
                    lstBufferCt=p.sqNo+1;
                }
                sendto(server, &ack, 100 , 0 , (struct sockaddr *) &relay1_addr,len );
                printf("SERVER S %s ACK %d SERVER RELAY1\n",get_current_time(),ack.sqNo);
            }
        }
        num_events = poll(server_poll, 1, 0);
    }
}