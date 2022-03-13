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
#include <dirent.h>

#define MAX 1024
#define PORT 8088
#define SA struct sockaddr


// char password[]="1234";

void sendfile(FILE *fp, int sockfd);
void writefile(int sockfd, FILE *fp);
ssize_t total=0;

char filepath[]="./media/test.mp4";

struct Args{

	int connfd;
};


void getUsernameData(){

	char const* const fileName = "logincred.txt"; /* should check that argc > 1 */
	FILE* file = fopen(fileName, "r"); /* should check the result */
	char line[500];

	while (fgets(line, sizeof(line), file)) {
		/* note that fgets don't strip the terminating \n, checking its
		   presence would allow to handle lines longer that sizeof(line) */
		printf("%s", line); 
		char username[500],password[500];
		bzero(username,sizeof(username));
		bzero(password,sizeof(password));
		 
		int i=0;
		while(line[i]!=','){
			username[i]=line[i]; i++;
		}
		username[i]='\0';
		while(line[i]!='\n'){
			password[i]=line[i] ;i++;
		}
		password[i]='\0';

		if(strcmp(username,rvd_username)==0){
			strcpy(curr_username,username);
			strcpy(curr_password,password);
			fclose(file);

			return 1;
		}

	}
	/* may check feof here to make a difference between eof and io failure -- network
	   timeout for instance */

	fclose(file);
	return 0;

}



void AuthUser(int connfd,int type,int logged_in[]){

	// type : 0 -> username , 1 -> password
	char buff[MAX]; 

	memset(buff,0,sizeof(buff));
	int read_id;


	read_id=read(connfd, buff, sizeof(buff));

	if(read_id<=0){
		printf("Client  | connection closed\n");
		pthread_exit(NULL);
	}

	if(type==0){ // user name
		

	}


	if(strcmp(buff,password)){
		memset(buff,0,sizeof(buff));

		if(type==0){
			strcpy(buff,"301");
		}else{
			strcpy(buff,"310");
		}

		write(connfd,buff,sizeof(buff));
		logged_in[0]=0;
		logged_in[1]=0;

	}else{
		memset(buff,0,sizeof(buff));

		if(type==0){
			strcpy(buff,"300");
		}else{
			strcpy(buff,"305");
		}
		
		write(connfd,buff,sizeof(buff));

		if(type==0) //username
		{
			logged_in[0]=1;
		}else{
			logged_in[1]=1;
		}

	}

}




void *cs_interaction(void *vargp){

	struct Args *args=(struct Args *)vargp;

	int connfd=args->connfd;

	char buff[MAX];


	int logged_in[2]={0,0};
	char curr_username[500];

	while(1){
		bzero(buff, sizeof(buff));
		
		// read the message from client and copy it in buffer
		if(read(connfd, buff, sizeof(buff))==0){
			printf("Client connection closed\n");
			pthread_exit(NULL);
		}

		if(strncmp("USERN",buff,5)==0){

			AuthUser(connfd,0,logged_in);

		}else if(logged_in[0] && strncmp("PASSWD",buff,6)==0){

			AuthUser(connfd,1,logged_in);

		}else if(logged_in[0] && logged_in[1] && strncmp("GetFile",buff,15)==0){
		
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

		}else if(logged_in[0] && logged_in[1] && strncmp(buff,"StoreFile",9)==0){


			printf("Trying to reveive the file \n");
			
			char filename[BUFFSIZE] = {0}; 
			if (recv(connfd, filename, BUFFSIZE, 0) == -1) 
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
			writefile(connfd, fp);
			printf("Receive Success, NumBytes = %ld\n", total);
			
			fclose(fp);


			continue;

		}else if(logged_in[0] && logged_in[1] && strncmp(buff,"ListDir",7)==0){
			char buffDir[1024];
			int l=0;
			DIR *d;
			struct dirent *dir;
			d = opendir(".");
			if (d) {
				while ((dir = readdir(d)) != NULL) {
					l+=snprintf(buffDir+l,1024,">| %s\n", dir->d_name);
				}
				closedir(d);
			}
			write(connfd,buffDir,sizeof(buffDir));

		}else if(strncmp("QUIT", buff, 3) == 0){
			printf("Client connection closed on request (QUIT) \n");
			pthread_exit(NULL);
		}else{
			bzero(buff,MAX);
			strcpy(buff,"[505] Command not supported");
			write(connfd,buff,sizeof(buff));
		}
		

	}



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