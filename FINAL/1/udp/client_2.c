
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX 1024
#define MAXLINE 1024
#define PORT 8083
#define SA struct sockaddr

struct Fruit{

	char fruit_name[100];
	int count;

};

void clear_input_stream(){
	char c;
	while ((c = getchar()) != '\n' && c != EOF) { }
}


void cs_interation(){

	char buff[MAX];
	struct Fruit fruit_buff;
	struct Fruit Fruits[5];

	int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;
   
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
   
    memset(&servaddr, 0, sizeof(servaddr));
       
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;
       
    int n, len;

	while(1){
		bzero(buff, sizeof(buff));
		printf("\nEnter the command : \n");
		n = 0;

		while ((buff[n++] = getchar()) != '\n');

		sendto(sockfd, (char *)buff, strlen(buff),
            MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
                sizeof(servaddr));

		if(strncmp(buff,"exit",4)==0){ // exit condition
			break;
		}

		if(strncmp(buff,"SendInventory",13)==0){

			printf("\nFruits\t\tcount\n");
			
			n = recvfrom(sockfd, (struct Fruit *)Fruits, MAXLINE, 
                        MSG_WAITALL, (struct sockaddr *) &servaddr,
                        &len);

			for(int i=0;i<5;i++){
				printf("%s\t\t%d\n",Fruits[i].fruit_name,Fruits[i].count);
			}
			continue;
		}



		bzero(buff, sizeof(buff));

		n = recvfrom(sockfd, (char *)buff, MAXLINE, 
                        MSG_WAITALL, (struct sockaddr *) &servaddr,
                        &len);

		buff[n] = '\0';

		printf("\nServer : %s\n", buff);

		if((strcmp(buff,"invalid choice")==0)){
			bzero(buff,sizeof(buff));
			continue;
		}

		printf("Enter the fruit name : \n");
		scanf("%s",fruit_buff.fruit_name);

		printf("Enter the count : \n");
		scanf("%d",&fruit_buff.count);

		// write(sockfd, &fruit_buff, sizeof(fruit_buff));

		sendto(sockfd, (struct Fruit *) &fruit_buff, sizeof(fruit_buff),
            MSG_CONFIRM, (const struct sockaddr *) &servaddr, 
                sizeof(servaddr));

		bzero(buff,sizeof(buff));

		n = recvfrom(sockfd, (char *)buff, MAXLINE, 
                        MSG_WAITALL, (struct sockaddr *) &servaddr,
                        &len);
		buff[n] = '\0';

		printf("\nserver : %s\n",buff);

		clear_input_stream();

	}
}



int main(){


	printf("\nCommands\n");
	printf("1 : Fruits\n");	
	printf("2 : SendInventory\n");
	printf("-------------------\n");


	cs_interation();

	return 0;
}