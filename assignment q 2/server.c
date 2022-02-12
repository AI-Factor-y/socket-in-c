#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include <fcntl.h> // for open
#include <unistd.h> // for close

#include "transfer.h"
#include <libgen.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <time.h>

#define MAX 100
#define PORT 8085
#define SA struct sockaddr

void sendfile(FILE *fp, int sockfd);
ssize_t total=0;

char filepath[]="./media/test.webm";



void cs_interaction(int connfd){

	char buff[MAX];


	while(1){
		bzero(buff, sizeof(buff));
		
		// read the message from client and copy it in buffer
		read(connfd, buff, sizeof(buff));


		if(strncmp("GivemeyourVideo",buff,15)==0){
		
			bzero(buff,MAX);

			printf("\n=> request for video recieved from client\n");

			char *filename = basename(filepath); 
			if (filename == NULL)
			{
				perror("Can't get filename");
				exit(1);
			}
			
			char buff[BUFFSIZE] = {0};
			strncpy(buff, filename, strlen(filename));
			if (send(connfd, buff, BUFFSIZE, 0) == -1)
			{
				perror("Can't send filename");
				exit(1);
			}
			
			FILE *fp = fopen(filepath, "rb");
			if (fp == NULL) 
			{
				perror("Can't open file");
				exit(1);
			}

			sendfile(fp, connfd);
			//puts("Send Success");

			printf("Send Success, NumBytes = %ld\n", total);
			fclose(fp);			

		}else if(strncmp("Bye", buff, 3) == 0){
			
		}else{
			bzero(buff,MAX);
			strcpy(buff,"We are online and ready");
			write(connfd,buff,sizeof(buff));
		}
		

	}



}

void sendfile(FILE *fp, int sockfd) 
{
	int n; 
	char sendline[MAX_LINE] = {0}; 
	total=0;
	char buff[100];
	clock_t t;
	t=clock();

	char *filename = "send_speed.txt";

    // open the file for writing
    FILE *send_fp = fopen(filename, "w");
    if (send_fp == NULL)
    {
        printf("Error opening the file %s", filename);
        return;
    }

   
	while ((n = fread(sendline, sizeof(char), MAX_LINE, fp)) > 0) 
	{
		total+=n;
		if (n != MAX_LINE && ferror(fp))
		{
			perror("Read File Error");
			exit(1);
		}
		t=clock();
		if (send(sockfd, sendline, n, 0) == -1)
		{
			perror("Can't send file");
			exit(1);
		}
		t=clock()-t;
		double time_taken = ((double)t)/CLOCKS_PER_SEC;

		printf("transfered %d bytes in %f second\n",n,time_taken);
		fprintf(send_fp,"%d %f\n",n,time_taken);

		recv(sockfd,buff,4,0);

		memset(sendline, 0, MAX_LINE);
	}
	// fclose(send_fp);

}



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
	connfd = accept(sockfd, (SA*)&cli, &len);
	if (connfd < 0) {
		printf("server accept failed...\n");
		exit(0);
	}
	else
		printf("server accept the client...\n");

	cs_interaction(connfd);
	
	// close the socket
	close(sockfd);


}




int main(){

	establish_connection();

	return 0;
}