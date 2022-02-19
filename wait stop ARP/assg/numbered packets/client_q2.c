#include<stdio.h>
#include<stdlib.h>

#include<sys/time.h>
#include<sys/wait.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h> 
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>

#define MAX 100
#define PORT 8083
#define SA struct sockaddr



void cs_interation(int sockfd){

	char buff[MAX];
	
	char ack_zero[10]="ack 0";
	char ack_one[10]="ack 1";

	int corrup_flag_1=0,corrup_flag_2=0;

	for(int i=0;i<10;i++){
		bzero(buff,sizeof(buff));

		printf("\n[.] wating for packet %d\n",i);
		int n=read(sockfd,buff,sizeof(buff));

		// if the send acklodgement is not recieved at the server
		// needed packet i but server send another packet (previous)
		if(buff[strlen(buff)-1]!=i+'0'){
			printf("\n[-] Discarding the packet %c (Packet already received) \n",buff[strlen(buff)-1]);
			i--;

			// sending acknowledgement for previous packet again
			// since it was lost

			if(i%2==0){
				write(sockfd,ack_one,sizeof(ack_one));
			}else{
				write(sockfd,ack_zero,sizeof(ack_zero));
			}

			continue; //  continue for the proper packet to be send
		}

		printf("[+] Server responded with : %s \n",buff);


		// simulation
		// client recieved and accepted the packet
		// but the acknowledgement got lost 
		if(i==4 && corrup_flag_1==0){
			corrup_flag_1=1;
			printf("\nSimulating : client recieved and accepted the packet\n");
			printf("but Acknoledgement got lost\n");
			continue;
		}

		// simulation : client sends the wrong ack to server
		if(i==8 && corrup_flag_2==0){
			corrup_flag_2=1;
			// i--;
			printf("\nSimulating : server recieves the wrong Acknoledgement number (delayed Acknoledgement)\n");
			if(i%2==1){
				write(sockfd,ack_one,sizeof(ack_one));
			}else{
				write(sockfd,ack_zero,sizeof(ack_zero));
			}

			continue;
		}

		if(i%2==0){
			write(sockfd,ack_one,sizeof(ack_one));
		}else{
			write(sockfd,ack_zero,sizeof(ack_zero));
		}

		printf("Acknoledgement sent\n");

	}

	close(sockfd);

}



void establish_connection(){

	int sockfd, connfd;
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
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);
   
	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");
   
	// function for chat
	cs_interation(sockfd);
   
	// close the socket
	close(sockfd);


}


int main(){

	establish_connection();
	return 0;
}