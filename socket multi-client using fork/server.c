#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>


#include <fcntl.h> // for open
#include <unistd.h> // for close
#define MAX 100
#define PORT 8085
#define SA struct sockaddr



struct Args{

	int connfd;
};


struct Arithm{
	int op[2];
	char operator_type;
};

int connection_i=0;	

struct Arithm split_string(char buff[MAX]){

	struct Arithm arith;

	int k=0;
	arith.op[0]=0;
	arith.op[1]=0;
	for(int i=0;buff[i]!='\n';i++){
		while(buff[i]!='\n' && buff[i]==' ') i++;
		
		if(buff[i]=='\n') break;

		if(buff[i]=='/'|| buff[i]=='+' || buff[i]=='-' || buff[i]=='*' || buff[i]==' '){
			k++;
			arith.operator_type=buff[i];
		}else{
			arith.op[k]=arith.op[k]*10+(buff[i]-'0');
		}
	}
	return arith;
}

int calc(struct Arithm arith){

	int res=0;

	if(arith.operator_type=='+'){
		res=arith.op[0]+arith.op[1];
	}else if(arith.operator_type=='-'){
		res=arith.op[0]-arith.op[1];
	}else if(arith.operator_type=='*'){
		res=arith.op[0]*arith.op[1];
	}else if(arith.operator_type=='/'){
		res=arith.op[0]/arith.op[1];
	}

	return res;

}


void cs_interaction(int connfd,int connection_id){


	printf("Connected with client %d\n\n",connection_id);

	char buff[MAX];

	while(1){

		bzero(buff, sizeof(buff));
		if(read(connfd, buff, sizeof(buff))==0){
			printf("Client connection closed\n");
			exit(0);
		}


		struct Arithm arith;

		arith=split_string(buff);

		printf("\n[Client%d]: %d %c %d",connection_id,arith.op[0],arith.operator_type,arith.op[1]);

		int res=calc(arith);

		printf("\n[Server]:%d\n",res);

		write(connfd, &res, sizeof(res));
	
	}

	
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
    
    pid_t childpid;
	// Accept the data packet from client and verification


	while(1){
		connfd = accept(sockfd, (SA*)&cli, &len);
		if (connfd < 0) {
			printf("server accept failed...\n");
			exit(0);
		}
		else
			printf("server accept the client...\n");

		connection_i++;

		if((childpid=fork())==0){

			close(sockfd);

			cs_interaction(connfd,connection_i);

		}
		
	}

	
	// close the socket
	close(sockfd);


}




int main(){

	establish_connection();

	return 0;
}