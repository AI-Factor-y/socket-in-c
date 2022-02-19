#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>

#include <fcntl.h> // for open
#include <unistd.h> // for close

#include<arpa/inet.h> 
#include<fcntl.h>
#include<sys/types.h>
#include<sys/time.h>
#include<netinet/in.h>

#define MAX 100
#define PORT 8083
#define SA struct sockaddr


struct Args{

    int connfd;
};


void *cs_interaction(void *vargp){

	struct Args *args=(struct Args *)vargp;

	int connfd=args->connfd;

	printf("new Client connected \n");

	char buff[MAX];
	
	time_t t1,t2;

	char msg[100]="server message : ";

	fd_set set;
	struct timeval timeout;
	int rv;

	int sim_flag=0;

	for(int i=0;i<10;i++){


		bzero(buff,sizeof(buff));

		msg[strlen(msg)-1]=i+'0';

		FD_ZERO(&set);
		FD_SET(connfd,&set);

		timeout.tv_sec=2;
		timeout.tv_usec=0;

		// simulating : server sends the packet to client but it does not reach client

		if(i==6 && sim_flag==0){
			printf("\nSimulation : server sends the packet to client but it does not reach client\n\n");
			sim_flag=1;
		}else{
			write(connfd,msg,sizeof(msg)); // sending the packet to the client
		}
		

		printf("\nMessage send to the client : %s\n",msg);

		// recieving the acknoledgement of the send packet from client

		rv=select(connfd+1,&set, NULL, NULL, &timeout);

		if(rv==-1){
			perror("select"); // some error occured
		}else if(rv==0){
			// a timeout occured , no acknoledgement received

			printf("\n[-] No acknowledgement recieved :> resending the packet\n\n");

			i--;
			continue;
		}

		// acknowledgement recieved 
		read(connfd,buff,sizeof(buff));

		// check if the acknowledgement is the correct ack for the send packet
		// checking for delayed acknowledgements

		// for packet 0 we should recieve an ack of 1
		if(buff[strlen(buff)-1]!=(i%2==0)+'0'){ 	
			printf("\n[-] acknowledgement problem\n");
			printf("ack is recieved but does not match with the current send packet\n");
			printf("resending current packet\n\n");
			i--;
			continue;
		}

		printf("[+] acknowledgement recieved and accepted : %s\n",buff);

	}


}

pthread_t thread_id[100];

void establish_connection(){

	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;
   
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));
   
	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);
   
	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");
   
	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	len = sizeof(cli);
   
	// Accept the data packet from client and verification
	int connection_i=0;	

	while(1){
		connfd = accept(sockfd, (SA*)&cli, &len);
		if (connfd < 0) {
			printf("server accept failed...\n");
			exit(0);
		}
		else
			printf("server accept the client...\n");

		
		struct Args args;
		args.connfd=connfd;
		

		pthread_create(&thread_id[connection_i++], NULL, cs_interaction, (void *) &args);
	
		if (connection_i >= 50) {
            // Update i
            connection_i = 0;
 
            while (connection_i < 50) {
                // Suspend execution of
                // the calling thread
                // until the target
                // thread terminates
                pthread_join(thread_id[connection_i++], NULL);
                
            }
 
            // Update i
            connection_i = 0;
        }
	}

	
	// close the socket
	close(sockfd);


}




int main(){


	establish_connection();

	return 0;
}