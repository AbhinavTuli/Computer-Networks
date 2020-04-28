#include "packet.h"

int main(int argc , char *argv[]) 
{ 
	// struct packet p;
	// printf("%d\n",sizeof(p));
	// return 0;
	srand(time(0));
    int lastSeqNoWritten=-(PACKET_SIZE-22);
    int isBuffered;
    struct packet bufferPkt;
    FILE*ptr=fopen("output.txt","w"); 
    
	int opt = TRUE; 
	int master_socket , addrlen , new_socket , client_socket[2] , activity, i , valread , sd; 
    int max_clients = 2 ;
	int max_sd; 
	struct sockaddr_in address; 		
	//set of socket descriptors 
	fd_set readfds; 
	
	//initialise all client_socket[] to 0 so not checked 
	for (i = 0; i < max_clients; i++){ 
		client_socket[i] = 0; 
	} 
		
	//create a master socket 
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0){ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	//set master socket to allow multiple connections , 
	//this is just a good habit, it will work without this 
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ){ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	
	//type of socket created 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
		
	//bind the socket to localhost port 8888 
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0){ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	//printf("Listener on port %d \n", PORT); 
		
	//try to specify maximum of 3 pending connections for the master socket 
	if (listen(master_socket, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
		
	//accept the incoming connection 
	addrlen = sizeof(address); 
	//puts("Waiting for connections ...");
	
	while(TRUE) { 
		//clear the socket set 
		FD_ZERO(&readfds); 
	
		//add master socket to set 
		FD_SET(master_socket, &readfds); 
		max_sd = master_socket; 
			
		//add child sockets to set 
		for ( i = 0 ; i < max_clients ; i++) 
		{ 
			//socket descriptor 
			sd = client_socket[i]; 
				
			//if valid socket descriptor then add to read list 
			if(sd > 0) 
				FD_SET( sd , &readfds); 
				
			//highest file descriptor number, need it for the select function 
			if(sd > max_sd) 
				max_sd = sd; 
		} 
	
		//wait for an activity on one of the sockets , timeout is NULL , 
		//so wait indefinitely 
		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL); 
	
		if ((activity < 0) && (errno!=EINTR)) 
		{ 
			printf("select error"); 
		} 
			
		//If something happened on the master socket , 
		//then its an incoming connection 
		if (FD_ISSET(master_socket, &readfds)) 
		{ 
			if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
			{ 
				perror("accept"); 
				exit(EXIT_FAILURE); 
			} 
			
			//inform user of socket number - used in send and receive commands 
			//printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
				
			//add new socket to array of sockets 
			for (i = 0; i < max_clients; i++) 
			{ 
				//if position is empty 
				if( client_socket[i] == 0 ) 
				{ 
					client_socket[i] = new_socket; 
					//printf("Adding to list of sockets as %d\n" , i); 
						
					break; 
				} 
			} 
		} 
			
		//else its some IO operation on some other socket 
		else{
            for (i = 0; i < max_clients; i++) { 
			sd = client_socket[i]; 
			if (FD_ISSET( sd , &readfds)) 
			{ 
				//Check if it was for closing , and also read the 
				//incoming message 
                struct packet p;
                // printf("is it here %d\n",i);
				if ((valread = read(sd , &p, PACKET_SIZE)) == 0) 
				{ 
					//Somebody disconnected , get his details and print 
					// getpeername(sd , (struct sockaddr*)&address ,(socklen_t*)&addrlen); 
					// printf("Host disconnected , ip %s , port %d \n" ,inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
						
					//Close the socket and mark as 0 in list for reuse 
					close( sd ); 
					client_socket[i] = 0; 
				} 
					
				else
				{ 
                    int flg=(rand()%100)+1; //will be a number from 1-100
					
                    if(flg>PDR){
                        if(p.sqNo-lastSeqNoWritten>PACKET_SIZE-22){ //means that some packet in between was dropped
                            isBuffered=1;
                            bufferPkt=p;
							printf("RCVD PKT: Seq. No %d of size %d Bytes from channel %d\n",bufferPkt.sqNo,bufferPkt.size,bufferPkt.chId);
							struct packet buffAck;
							buffAck.sqNo=bufferPkt.sqNo;
							buffAck.chId=bufferPkt.chId;
							buffAck.isData=0;
							send(sd , &buffAck , PACKET_SIZE , 0 ); 
							printf("SENT ACK: for PKT with Seq. No. %d from channel %d\n",buffAck.sqNo,buffAck.chId);
                        }
                        else{
                            fflush(ptr);
                            fprintf(ptr,"%s",p.payload);
                            if(p.isLast==1){
                                fflush(ptr);
                                fprintf(ptr,"%s",p.payload);
                            }
                            printf("RCVD PKT: Seq. No %d of size %d Bytes from channel %d\n",p.sqNo,p.size,p.chId);
                            lastSeqNoWritten=p.sqNo;
                            struct packet ack;
                            ack.sqNo=p.sqNo;
                            ack.chId=p.chId;
                            ack.isData=0;
                            send(sd , &ack , PACKET_SIZE , 0 ); 
                            printf("SENT ACK: for PKT with Seq. No. %d from channel %d\n",ack.sqNo,ack.chId);

                            if(isBuffered==1){
                                fflush(ptr);
                                fprintf(ptr,"%s",bufferPkt.payload);
                                if(bufferPkt.isLast==1){
                                    fflush(ptr);
                                    fprintf(ptr,"%s",bufferPkt.payload);
                                }
                                lastSeqNoWritten=bufferPkt.sqNo;
                                isBuffered=0;
                            }

                        }
                    }
                    // else{ //otherwise it drops
                    //     printf("PKT DROPPED: for PKT with Seq. No. %d from channel %d\n",p.sqNo,p.chId);
                    // }
                    
				} 
			} }
		} 
	} 
    fclose(ptr);
	return 0; 
} 

