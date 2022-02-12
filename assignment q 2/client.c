
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#include <fcntl.h> // for open
#include <unistd.h> // for close

#include "transfer.h"
#include <netinet/in.h>
#include <time.h>

#define MAX 100
#define PORT 8085
#define SA struct sockaddr

void writefile(int sockfd, FILE *fp);
ssize_t total=0;


void clear_input_stream(){
	char c;
	while ((c = getchar()) != '\n' && c != EOF) { }
}


void cs_interation(int sockfd){

	char buff[MAX];
	int n;


	while(1){
		bzero(buff, sizeof(buff));
		printf("\nEnter the command : \n");
		n = 0;

		while ((buff[n++] = getchar()) != '\n');

		write(sockfd, buff, sizeof(buff));

		if(strncmp(buff,"Bye",3)==0){ // exit condition
			break;
		}

		if(strncmp(buff,"GivemeyourVideo",15)==0){

			printf("Trying to reveive the file \n");
			
			char filename[BUFFSIZE] = {0}; 
			if (recv(sockfd, filename, BUFFSIZE, 0) == -1) 
			{
				perror("Can't receive filename");
				exit(1);
			}

			FILE *fp = fopen(filename, "wb");
			if (fp == NULL) 
			{
				perror("Can't open file");
				exit(1);
			}
			
			char addr[INET_ADDRSTRLEN];
			// printf("Start receive file: %s from %s\n", filename, inet_ntop(AF_INET, &clientaddr.sin_addr, addr, INET_ADDRSTRLEN));
			writefile(sockfd, fp);
			printf("Receive Success, NumBytes = %ld\n", total);
			
			fclose(fp);


			continue;
		}


		bzero(buff, sizeof(buff));

		read(sockfd, buff, sizeof(buff));

		printf("\nServer : %s\n", buff);

		// clear_input_stream();

	}
}

void writefile(int sockfd, FILE *fp)
{
	ssize_t n;
	char ack_buff[100]={0};
	char buff[MAX_LINE] = {0};
	total=0;
	clock_t t;
	char *filename = "receive_speed.txt";

    // open the file for writing
    FILE *receive_fp = fopen(filename, "w");
    if (receive_fp == NULL)
    {
        printf("Error opening the file %s", filename);
        return;
    }


	t=clock();
	while ((n = recv(sockfd, buff, MAX_LINE, 0)) > 0) 
	{	
		t=clock()-t;
		double time_taken = ((double)t)/CLOCKS_PER_SEC;

		printf("received %ld bytes in %f second\n",n,time_taken);
		fprintf(receive_fp,"%ld %f\n",n,time_taken);

		strcpy(ack_buff,"rcvd");
		send(sockfd,ack_buff,4,0);

		total+=n;
		if (n == -1)
		{
			perror("Receive File Error");
			exit(1);
		}
		

		if (fwrite(buff, sizeof(char), n, fp) != n)
		{
			perror("Write File Error");
			exit(1);
		}

		if(n<MAX_LINE) break;

		memset(buff, 0, MAX_LINE);
	}

	fclose(receive_fp);
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


	// printf("\nCommands\n");
	// printf("1 : Fruits\n");	
	// printf("2 : SendInventory\n");
	// printf("-------------------\n");


	establish_connection();

	return 0;
}