#include "packet.h"

void insertInWindow(struct windowNode* head, struct windowNode*current){
    if(head==NULL){
        head=current;
    }
    else{
        struct windowNode* temp=head;
        while(temp->next!=NULL){
            temp=temp->next;
        }
        temp->next=current;
    }
    return;
}

struct windowNode* findSeqNoInWindow(struct windowNode* head,int sqNo){
    struct windowNode* temp=head;
    while(temp!=NULL){
        if(temp->pkt.sqNo==sqNo){
            return temp;
        }
        temp=temp->next;
    }
    return NULL;
}

int findCurrentWindowSize(struct windowNode* head){
    if(head==NULL){
        return 0;
    }
    struct windowNode* temp=head;
    int ct=0;
    while(temp!=NULL){
        ct++;
        temp=temp->next;
    }
    return ct;
}

int main(){
    FILE *fp = fopen("input.txt","rb"); 
    if(fp==NULL){
        printf("File open error");
        return 1; 
    }
    size_t bytesRead = 0;

    int client,valread; 
    client=0;
	struct sockaddr_in relay2_addr; 
    struct sockaddr_in relay1_addr; 
    struct sockaddr_in client_addr; 
    struct sockaddr_in incoming_addr;
    int len=sizeof(relay1_addr);
	if ((client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) 
	{ 
		printf("\n Error while creating socket! \n"); 
		return -1; 
	} 

    client_addr.sin_family = AF_INET; 
	client_addr.sin_port = htons(PORT3);  

	relay1_addr.sin_family = AF_INET; 
	relay1_addr.sin_port = htons(PORT0); 

    relay2_addr.sin_family = AF_INET; 
	relay2_addr.sin_port = htons(PORT1); 
    
	// Converting address to binary form 
	if(inet_pton(AF_INET, "127.0.0.1", &relay2_addr.sin_addr)<=0) 
	{ 
		printf("\nAddress is Invalid!\n"); 
		return -1; 
	} 
    if(inet_pton(AF_INET, "127.0.0.1", &relay1_addr.sin_addr)<=0) 
	{ 
		printf("\nAddress is Invalid!\n"); 
		return -1; 
	} 
    if(inet_pton(AF_INET, "127.0.0.1", &client_addr.sin_addr)<=0) 
	{ 
		printf("\nAddress is Invalid!\n"); 
		return -1; 
	} 

     if( bind(client , (struct sockaddr*)&client_addr, len ) == -1) {
        printf("\nError while binding!\n"); 
		return -1; 
    }

    char buffer[PACKET_SIZE-10];  
    int sqNo=0;
    bytesRead=fread(buffer, 1, sizeof(buffer), fp);
    bool isLastAcked=false;

    struct pollfd client_poll[1];
    client_poll[0].fd=client;
    client_poll[0].events=POLLIN;
    int num_events;
    num_events=0;
    struct windowNode* head=NULL;
    int canSend=4; //number of packets that can be sent rightnow
    clock_t start;
    int fl=0;
    while((!isLastAcked)){
        if(fl==1){
            clock_t curr=clock();
            int diff=(curr-start)* 1000 / CLOCKS_PER_SEC; // gives difference in milliseconds
            if(diff>3){
                struct packet resend;
                resend=head->pkt;
                //printf("Timed out waiting for %d\n",resend.sqNo);
                if(resend.sqNo%2==0){
                    if (sendto(client, &resend, 100 , 0 , (struct sockaddr *) &relay2_addr,len )==-1){
                        printf("\nSendto Error\n"); 
                        return -1; 
                    }
                    //printf("SEG1\n");
                    printf("CLIENT S %s DATA %d CLIENT RELAY2\n",get_current_time(),resend.sqNo);
                }
                else{
                    if (sendto(client, &resend, 100 , 0 , (struct sockaddr *) &relay1_addr,len )==-1){
                        printf("\nSendto Error\n"); 
                        return -1; 
                    }
                    //printf("SEG2\n");
                    printf("CLIENT S %s DATA %d CLIENT RELAY1\n",get_current_time(),resend.sqNo);
                }
                start=clock();


            }
            
        }
        fl=1;

        if(bytesRead>0 && canSend>0){
            struct windowNode* current=malloc(sizeof(struct windowNode));
            struct packet p;
            if(bytesRead<PACKET_SIZE-10){
                buffer[bytesRead]='\0';
            }
            memcpy(p.payload, buffer, sizeof(buffer)); // as not ending with \0, so we use this instead of strcpy
            p.size=bytesRead;
            p.sqNo=sqNo;
            p.isData=1;
            p.isLast=0;
            sqNo+=1;
            sqNo=sqNo%8;
            bytesRead=fread(buffer, 1, sizeof(buffer), fp);
            if(bytesRead==0){
                p.isLast=1;
            }
            current->pkt=p;
            if(head==NULL){
                head=current;
                start=clock();
            }
            else{
                struct windowNode* temp=head;
                while(temp->next!=NULL){
                    temp=temp->next;
                }
                temp->next=current;
            }
            canSend=4-findCurrentWindowSize(head);

            if(p.sqNo%2==0){
                if (sendto(client, &p, 100 , 0 , (struct sockaddr *) &relay2_addr,len )==-1){
                    printf("\nSendto Error\n"); 
                    return -1; 
                }
                printf("CLIENT S %s DATA %d CLIENT RELAY2\n",get_current_time(),p.sqNo);
            }
            else{
                if (sendto(client, &p, 100 , 0 , (struct sockaddr *) &relay1_addr,len )==-1){
                    printf("\nSendto Error\n"); 
                    return -1; 
                }
                printf("CLIENT S %s DATA %d CLIENT RELAY1\n",get_current_time(),p.sqNo);
            }
        }

        while(num_events>0){
            struct packet ack;
            recvfrom(client,&ack,PACKET_SIZE,0,(struct sockaddr *) &incoming_addr, &len);
            num_events--;
            struct windowNode* found=findSeqNoInWindow(head,ack.sqNo);
            if(found==NULL){
                printf("Duplicate ack!! %d\n",ack.sqNo);
                return -1;
            }
            found->isAcked=1;
            while(head!=NULL && head->isAcked==1){ //repositioning window head
                head=head->next;
                start=clock();
            }
            canSend=4-findCurrentWindowSize(head);
            printf("CLIENT R %s ACK %d RELAY1 CLIENT\n",get_current_time(),ack.sqNo);
            if(ack.isLast){
                isLastAcked=true;
            }
            //canSend
        }
        num_events = poll(client_poll, 1, 0);
        //usleep(50);
    }

    close(client);
    return 0;
}
    