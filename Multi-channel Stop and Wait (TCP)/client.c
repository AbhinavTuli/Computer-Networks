#include "packet.h"

int main(){
    FILE *fp = fopen("input.txt","rb"); 
    if(fp==NULL){
        printf("File open error");
        return 1; 
    }
    char buffer[PACKET_SIZE-22];  
    size_t bytesRead = 0;

    int sock0,sock1,valread; 
    sock0=sock1=0;
	struct sockaddr_in serv_addr; 

	if ((sock0 = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Error while creating socket! \n"); 
		return -1; 
	} 

    if ((sock1 = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Error while creating socket! \n"); 
		return -1; 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
    
	// Converting address to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nAddress is Invalid!\n"); 
		return -1; 
	} 

	if (connect(sock0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	} 
    if (connect(sock1, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	} 


    struct pollfd pfds0[1];
    struct pollfd pfds1[1];
    pfds0[0].fd=sock0;
    pfds0[0].events=POLLIN;
    pfds1[0].fd=sock1;
    pfds1[0].events=POLLIN;

    int num_events0,num_events1;
    num_events0=num_events1=0;

    int sqNo=0;
    int oddEven=0; //0 for even packet to be sent through sock0 and 1 for odd packet sock1

    int num0,num1;

    struct packet prev0; //last packet from channel 0 buffered
    struct packet prev1; //last packet from channel 1 buffered
    
    bytesRead=fread(buffer, 1, sizeof(buffer), fp);

    while (bytesRead > 0 || num_events0==0 || num_events1==0)
    {
        if(sqNo!=0 && sqNo!=PACKET_SIZE-22){ //check for previous acks if not the first packet sent from that channel
            if(oddEven==0){
                while(num_events0==0){
                        send(sock0 , &prev0 , PACKET_SIZE , 0 ); 
                        printf("SENT PKT: Seq. No %d of size %d Bytes from channel %d\n",prev0.sqNo,prev0.size,prev0.chId);
                        num_events0 = poll(pfds0, 1, TMOUT);
                    }
                struct packet ack;
                num0=recv(sock0, &ack, PACKET_SIZE, 0);
                printf("RCVD ACK: for PKT with Seq. No. %d from channel %d\n",ack.sqNo,ack.chId);
                num_events0=-1;
            }
            else{
                while(num_events1==0){
                        send(sock1 , &prev1 , PACKET_SIZE , 0 ); 
                        printf("SENT PKT: Seq. No %d of size %d Bytes from channel %d\n",prev1.sqNo,prev1.size,prev1.chId);
                        num_events1 = poll(pfds1, 1, TMOUT);
                    }
                struct packet ack;
                num1=recv(sock1, &ack, PACKET_SIZE, 0);
                printf("RCVD ACK: for PKT with Seq. No. %d from channel %d\n",ack.sqNo,ack.chId);
                num_events1=-1;
            }
        }
        if(bytesRead==0){
            oddEven=1-oddEven;
            continue;
        }
        if(bytesRead<PACKET_SIZE-22){
            buffer[bytesRead]='\0';
        }
        

        struct packet p;
        memcpy(p.payload, buffer, sizeof(buffer)); // as not ending with \0, so we use this instead of strcpy
        p.size=bytesRead;
        p.sqNo=sqNo;
        p.isData=1;
        //p.chId=0;
        p.isLast=0;
        sqNo+=bytesRead;
        bytesRead=fread(buffer, 1, sizeof(buffer), fp);
        if(bytesRead==0){
            p.isLast=1;
        }
        if(oddEven==0){
            p.chId=0;
            prev0=p;
            send(sock0 , &p , PACKET_SIZE , 0 ); 
            printf("SENT PKT: Seq. No %d of size %d Bytes from channel %d\n",p.sqNo,p.size,p.chId);
        }
        else{
            p.chId=1;
            prev1=p;
            send(sock1 , &p , PACKET_SIZE , 0 ); 
            printf("SENT PKT: Seq. No %d of size %d Bytes from channel %d\n",p.sqNo,p.size,p.chId);
        }
        if(oddEven==0){
            num_events0 = poll(pfds0, 1, TMOUT);
        }
        else
        {
            num_events1 = poll(pfds1, 1, TMOUT);
        }        
        oddEven=1-oddEven;  //this line changes 0 to 1 and 1 to 0
    }
    if(num_events0!=-1){
        struct packet ack;
        num0=recv(sock0, &ack, PACKET_SIZE, 0);
        printf("RCVD ACK: for PKT with Seq. No. %d from channel %d\n",ack.sqNo,ack.chId);
    }
    if(num_events1!=-1){
        struct packet ack;
        num1=recv(sock1, &ack, PACKET_SIZE, 0);
        printf("RCVD ACK: for PKT with Seq. No. %d from channel %d\n",ack.sqNo,ack.chId);
    }
    close(sock1);
    close(sock0);
    return 0;
}
    