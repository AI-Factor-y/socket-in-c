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

struct Fruit Fruits[5];



void setFruits(){

	strcpy(Fruits[0].fruit_name,"apple");
	strcpy(Fruits[1].fruit_name,"mango");
	strcpy(Fruits[2].fruit_name,"banana");
	strcpy(Fruits[3].fruit_name,"chikoo");
	strcpy(Fruits[4].fruit_name,"papaya");

	for(int i=0;i<5;i++){
		Fruits[i].count=5;
	}

}



void cs_interaction(){

	struct Fruit fruit_buff;

	int sockfd;
    char buff[MAXLINE];
    char send_buff[1024];

    struct sockaddr_in servaddr, cliaddr;
       
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
       
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));
       
    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
       
    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr, 
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
       
    int len, n;



	while(1){
		bzero(buff, sizeof(buff));
		
		// read the message from client and copy it in buffer
		len = sizeof(cliaddr);  //len is value/resuslt
        
        n = recvfrom(sockfd, (char *)buff, MAXLINE, 
                    MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                    &len);
        buff[n] = '\0';


		if(strncmp("Fruits",buff,6)==0){
		
			bzero(buff,MAX);

			printf("\n=> request for fruits recieved from client\n");

			strcpy(buff,"Enter the name of the fruit");

			sendto(sockfd, (char *)buff, strlen(buff), 
            MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
                len);

			n = recvfrom(sockfd, (struct Fruit *) &fruit_buff, MAXLINE, 
                    MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                    &len);
	   

			printf("fruit requested : %s count requested : %d\n",fruit_buff.fruit_name,fruit_buff.count);
			
			bzero(buff,MAX);

			int found=0;

			for(int i=0;i<5;i++){
				if(strcmp(fruit_buff.fruit_name,Fruits[i].fruit_name)==0){
					if(fruit_buff.count>Fruits[i].count){
						
						strcpy(buff,"Not available");
						sendto(sockfd, (char *)buff, strlen(buff), 
			            MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
			                len);
					
					}else{

						Fruits[i].count-=fruit_buff.count;
						strcpy(buff,"Request has been processed");
						sendto(sockfd, (char *)buff, strlen(buff), 
			            MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
			                len);

					}
					found=1;
					break;
				}
			}

			if(!found){
				strcpy(buff,"this fruit is not sold here");
				sendto(sockfd, (char *)buff, strlen(buff), 
	            MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
	                len);
			}

		}else if(strncmp("SendInventory", buff, 13) == 0){
			bzero(buff,MAX);

			printf("\n=> request for inventory recieved from client\n");

			// here
			sendto(sockfd, (struct Fruits *)Fruits, sizeof(Fruits), 
            MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
                len);

		}else if(strncmp("exit", buff, 4) == 0){

			printf("Server Exit...\n");
			break;

		}else{
			bzero(buff,MAX);
			strcpy(buff,"invalid choice");
			sendto(sockfd, (char *)buff, strlen(buff), 
	            MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
	                len);
		}
		

	}



}



int main(){

	setFruits();

	cs_interaction();

	return 0;
}