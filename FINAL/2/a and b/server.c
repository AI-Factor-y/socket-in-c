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

#include <pthread.h>
#include <time.h>

#define MAX 100
#define PORT 8088
#define SA struct sockaddr

void sendfile(FILE *fp, int sockfd);
ssize_t total=0;

char filepath[]="./media/test.mp4";

struct Args{

    int connfd;
};


void *cs_interaction(void *vargp){

	struct Args *args=(struct Args *)vargp;

	int connfd=args->connfd;

	char buff[MAX];


	while(1){
		bzero(buff, sizeof(buff));
		
		// read the message from client and copy it in buffer
		if(read(connfd, buff, sizeof(buff))==0){
			printf("Client connection closed\n");
			pthread_exit(NULL);
		}


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
			printf("Client connection closed on request (BYE) \n");
			pthread_exit(NULL);
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
    double speed;

    clock_t time_start=clock();
    clock_t time_now;

    float gap=0.1;

   	double curr_t_limit=gap;

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

		speed = (float)n/time_taken;


		time_now=clock()-time_start;
		double time_from_start = ((double)time_now)/CLOCKS_PER_SEC;

		if(time_from_start>=curr_t_limit){

			fprintf(send_fp,"%f %f\n",curr_t_limit,speed);
			printf("%f | transfered %d bytes in %f second | transfer speed : %f\n",curr_t_limit,n,time_taken,speed);
			curr_t_limit+=gap;
		}

		// printf("transfered %d bytes in %f second | transfer speed : %f\n",n,time_taken,speed);

		// fprintf(send_fp,"%d %f\n",n,time_taken);

		recv(sockfd,buff,4,0);

		memset(sendline, 0, MAX_LINE);
	}
	fclose(send_fp);

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