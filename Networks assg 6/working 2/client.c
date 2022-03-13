
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <libgen.h>

#include "transfer.h"
#include <netinet/in.h>
#include <time.h>

#define MAX 1024
#define PORT 8081
#define SA struct sockaddr

void writefile(int sockfd, FILE *fp);
ssize_t total=0;

void sendfile(FILE *fp, int sockfd);



char filepath[]="./media/cltest.mp4";

void clear_input_stream(){
	char c;
	while ((c = getchar()) != '\n' && c != EOF) { }
}


// athorisation section --------------------------------------------------------
void AuthUser(int sockfd,int type){

	// sending username / password
	char buff[MAX];
	char password[MAX];

	if(type==0){
		printf("Enter the Username : ");
	}else{
		printf("Enter the password : ");
	}
	
	scanf("%s",password);
	send(sockfd,password,MAX,0);

	memset(buff,0,sizeof(buff));	

}
//________________________________________________________________________________

// File transfer section (send file)----------------------------------------------

void sendFileMain(int sockfd){

	printf("\n=> sending video to server\n");

	printf("%s\n",filepath);
	char *filename = basename(filepath); 

	if (filename == NULL)
	{
		perror("Can't get filename");
		exit(1);
	}
	
	char buff[BUFFSIZE] = {0};
	strncpy(buff, filename, strlen(filename));
	if (send(sockfd, buff, BUFFSIZE, 0) == -1)
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

	sendfile(fp, sockfd);
	//puts("Send Success");

	printf("Send Success, NumBytes = %ld\n", total);
	fclose(fp);

}

void sendfile(FILE *fp, int sockfd) 
{
	int n; 
	char sendline[MAX_LINE] = {0}; 
	total=0;
	char buff[MAX];
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

//________________________________________________________________________

// file transfer section ( revieve file)----------------------------------


void recieveFileMain(int sockfd){
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

}



void writefile(int sockfd, FILE *fp)
{
	ssize_t n;
	char ack_buff[MAX]={0};
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

	double speed;

    clock_t time_start=clock();
    clock_t time_now;

    float gap=0.1;

   	double curr_t_limit=gap;


	while ((n = recv(sockfd, buff, MAX_LINE, 0)) > 0) 
	{	
		t=clock()-t;
		double time_taken = ((double)t)/CLOCKS_PER_SEC;
		speed = (float)n/time_taken;

		time_now=clock()-time_start;
		double time_from_start = ((double)time_now)/CLOCKS_PER_SEC;

		if(time_from_start>=curr_t_limit){

			fprintf(receive_fp,"%f %f\n",curr_t_limit,speed);
			printf("%f | received %ld bytes in %f second | transfer speed : %f\n",curr_t_limit,n,time_taken,speed);
			curr_t_limit+=gap;
		}


		// printf("received %ld bytes in %f second | transfer speed : %f\n",n,time_taken,speed);
		// fprintf(receive_fp,"%ld %f\n",n,time_taken);

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
//___________________________________________________________________________




void cs_interation(int sockfd){

	char buff[MAX];
	int n;

	int logged_in[2]={0,0};
	int authFlag=0;
	while(1){
		authFlag=0;
		bzero(buff, sizeof(buff));
		printf("\nEnter the command : \n");
		n = 0;

		while ((buff[n++] = getchar()) != '\n');

		write(sockfd, buff, sizeof(buff));

		// login section ahead

		if(strncmp(buff,"USERN",5)==0){
			AuthUser(sockfd,0);
			authFlag=1;
		}


		if(logged_in[0] && strncmp(buff,"PASSWD",6)==0){
			AuthUser(sockfd,1);
			authFlag=1;
		}


		if(strncmp(buff,"QUIT",3)==0){ // exit condition
			break;
		}

		if(logged_in[0] && logged_in[1] && strncmp(buff,"GetFile",7)==0){
			printf("Enter filename to get : ");

			bzero(buff,sizeof(buff));
			n = 0;

			while ((buff[n++] = getchar()) != '\n');
			buff[n-1]='\0';
			write(sockfd, buff, sizeof(buff));

			recieveFileMain(sockfd);
			continue;
		}

		if(logged_in[0] && logged_in[1] && strncmp(buff,"StoreFile",9)==0){

			bzero(buff,sizeof(buff));

			printf("Enter the filename to store : ");

			n = 0;
			while ((buff[n++] = getchar()) != '\n');
			buff[n-1]='\0';
			strcpy(filepath,buff);
			sendFileMain(sockfd);

			continue;
		}

		if(logged_in[0] && logged_in[1] && strncmp(buff,"CreateFile",10)==0){

			bzero(buff,sizeof(buff));

			printf("Enter the filename to store : ");

			n = 0;
			while ((buff[n++] = getchar()) != '\n');
			buff[n-1]='\0';
			
			write(sockfd, buff, sizeof(buff));

			continue;
		}

		bzero(buff, sizeof(buff));

		read(sockfd, buff, sizeof(buff));


		if(strcmp(buff,"300")==0){
			printf("\n[300] Correct Username; Need password\n");
			logged_in[0]=1;

		}else if(strcmp(buff,"301")==0){
			printf("[301] Incorrect Username\n");
			logged_in[0]=0; logged_in[1]=0;

		}else if(strcmp(buff,"305")==0){
			printf("[305] User Authenticated with password\n");
			logged_in[1]=1;

		}else if(strcmp(buff,"310")==0){
			printf("[310] Incorrect password\n");
			logged_in[0]=0; logged_in[1]=0;
		}else{
			printf("\n%s\n", buff);
		}

		if(authFlag) clear_input_stream();

	}
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