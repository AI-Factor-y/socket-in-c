
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>


#include <fcntl.h> // for open
#include <unistd.h> // for close

#define MAX 100
#define PORT 8085
#define SA struct sockaddr


void clear_input_stream(){
	char c;
	while ((c = getchar()) != '\n' && c != EOF) { }
}


void cs_interation(int sockfd){

	char buff[MAX];
	int n=0;

	while(1){
		bzero(buff, sizeof(buff));

		printf("\nenter the query: ");
		n = 0;

		while ((buff[n++] = getchar()) != '\n');


		write(sockfd, buff, sizeof(buff));

		int res=0;
		read(sockfd, &res, sizeof(res));

		printf("\n[Server]: %d\n",res);

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
		printf("connected to the server\n");
   
	// function for chat
	cs_interation(sockfd);
   
	// close the socket
	close(sockfd);

}


int main(){

	establish_connection();

	return 0;
}