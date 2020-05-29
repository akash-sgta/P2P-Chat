# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <pthread.h>
# include <errno.h>

# define BUFFER_SZ 1024

typedef struct MAINTAIN{
	int port;
	char ip[100];
	char name[100];
}maintain;
maintain list [100];

int ind = 0;
int sockfd, n;

char rec_buffer[BUFFER_SZ];
char send_buffer[BUFFER_SZ];
char name[100];

int *ptr[2];
pthread_t tid[2];

pthread_mutex_t clients_m = PTHREAD_MUTEX_INITIALIZER;

void test(char *arg, int flag, int line){
	char *f = (flag==1)?"rec_buffer":"send_buffer";
	printf("\n\n[-]\t%d\t%s\t-\t%s\n\n", line, f, arg);
}

void error(char *str, int flag){
	perror(str);
	if(flag)
		exit(flag);
}

void *keep_receiving(){
	
	while(1){
		
		bzero(rec_buffer, BUFFER_SZ);
		recv(sockfd, &rec_buffer, sizeof(rec_buffer), MSG_PEEK);
		
		if(strlen(rec_buffer) == 0)
			continue;

		bzero(rec_buffer, BUFFER_SZ);
		recv(sockfd, &rec_buffer, sizeof(rec_buffer), 0);
		printf("%s\n", rec_buffer);
		
		if(strstr(rec_buffer, ":end:") != 0){
			
			bzero(send_buffer, BUFFER_SZ);
			strcpy(send_buffer, ":die:");
			send(sockfd, &send_buffer, sizeof(send_buffer), 0);

			if(pthread_cancel(tid[1]) == 0){
				printf("[T] keep_receiving  -(cancelled)->  keep_sending\n[.] PRESS : Ctrl+C\n[.] Restart the client and please use the same name\n[.] Thank You\n");
				//Cound not resolve why its not going back to the menu after this command
			}else
				printf("[T] keep_receiving failed\n");
			
			int ret = 0;
			pthread_exit(&ret);
		}
		
	}

}

void *keep_sending(){
	
	char buffer[900];
	int i;
	
	while(1){

		bzero(send_buffer, BUFFER_SZ);
		scanf("%[^\n]", buffer);
		getchar();
		
		if(strlen(buffer) == 0)
			continue;
		
		for(i=0; i<900; i++){
			if(buffer[i] == ' ')
				buffer[i] = '_';
			if(buffer[i] == '\n' || buffer[i] == '\0'){
				buffer[i] = '\0';
				break;
			}
			
		}
		
		sprintf(send_buffer, "[%s] -> { %s }", name, buffer);
		send(sockfd, &send_buffer, sizeof(send_buffer), 0);

		if(strstr(send_buffer, ":end:") != 0){
			if(pthread_cancel(tid[0]) == 0)
				printf("[T] keep_sending  -(cancelled)-> keep_receiving\n");
			else
				printf("[T] keep_sending failed\n");
			
			int ret = 0;
			pthread_exit(&ret);
		}
	}
}

void update_list(){
	
	bzero(send_buffer, BUFFER_SZ);
	strcpy(send_buffer, "request");
	send(sockfd, &send_buffer, sizeof(send_buffer), 0);
	
	int num;
	
	bzero(rec_buffer, BUFFER_SZ);
	recv(sockfd, &rec_buffer, sizeof(rec_buffer), 0);
	num = atoi(rec_buffer);
	ind = 0;

	printf("\n------------------------------------------------------------------------\n");
	printf("\t[ID]-----:-----[IP]-----:-----[PORT]-----:-----[NAME]\n");
	
	maintain *temp;
	
	while(num--){
		
		temp = (maintain*)malloc(sizeof(maintain));
		
		bzero(rec_buffer, BUFFER_SZ);
		recv(sockfd, &rec_buffer, sizeof(rec_buffer), 0);
		
		strcpy(temp->ip, rec_buffer);
		
		bzero(rec_buffer, BUFFER_SZ);
		recv(sockfd, &rec_buffer, sizeof(rec_buffer), 0);
		
		temp->port = atoi(rec_buffer);
		
		bzero(rec_buffer, BUFFER_SZ);
		recv(sockfd, &rec_buffer, sizeof(rec_buffer), 0);
		
		strcpy(temp->name, rec_buffer);
		
		strcpy(list[ind].ip, temp->ip);
		strcpy(list[ind].name, temp->name);
		list[ind].port = temp->port;
		printf("\t[%d]-----:-----[%s]-----:-----[%d]-----:-----[%s]\n", ind, temp->ip, temp->port, temp->name);
		ind += 1;
		
		free(temp);
		
	}
	printf("\n------------------------------------------------------------------------\n");
}

int main(int argc, char *argv[]){
	
	if(argc < 3)
		error("[x] ERROR : USAGE -> filename ip_address port_number\n", 1);
	
	struct sockaddr_in server;
	int flag = 1, i;

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	server.sin_family = AF_INET;
	server.sin_port = atoi(argv[2]);
	server.sin_addr.s_addr=inet_addr(argv[1]);
	
	n = sizeof(server);

	if(connect(sockfd, (struct sockaddr *)&server, n) < 0)
		error("[x] ERROR : Connection failure\n", 1);
	
	printf("\n");
	printf("****************************************\n");
	printf("*                                      *\n");
	printf("*     WELCOME TO PEER TO PEER CHAT     *\n");
	printf("*           Language used : C          *\n");
	printf("*     For Operating System : Linux     *\n");
	printf("*       Author : Akash Sengupta        *\n");
	printf("*     Github repository : nibbax64     *\n");
	printf("*             ---CLIENT---             *\n");
	printf("*                                      *\n");
	printf("****************************************\n\n");

	printf("\n***** PLEASE DO NOT USE CTRL+C OR ANY OTHER SYSTEM EXIT COMMAND UNLESS EXTREMELY NECESSARY *****\n");
	
	printf("[.] Client side has been setup successfully...\n");
	
	bzero(send_buffer, BUFFER_SZ);
	printf("[.] USER_NAME : ");
	scanf("%[^\n]", name);
	for(i=0; i<100; i++){
		if(name[i] == ' ')
			name[i] = '_';
		if(name[i] == '\n' || name[i] == '\0'){
			name[i] = '\0';
			break;
		}
	}
	strcpy(send_buffer, name);
	send(sockfd, &send_buffer, sizeof(send_buffer), 0);
	
	while(flag){
		
		printf("\n");
		printf("[1] Get the list of clients\n");
		printf("[2] Connect to a client\n");
		printf("[3] Wait for connection\n");
		printf("[4] Logout\n\n[.] > ");
		
		int dec, conn;
		scanf("%d", &dec);

		switch(dec){
			case 1:{
				update_list();
			}break;
			case 2:{
				update_list();
				
				printf("[.] Enter the id of client you want to connect to : ");

				scanf("%d", &conn);

				if(conn < ind){
					
					bzero(send_buffer, BUFFER_SZ);
					strcpy(send_buffer, "connect");
					send(sockfd, &send_buffer, sizeof(send_buffer), 0);
					
					bzero(send_buffer, BUFFER_SZ);
					strcpy(send_buffer, list[conn].ip);
					send(sockfd, &send_buffer, sizeof(send_buffer), 0);
				
					bzero(send_buffer, BUFFER_SZ);
					sprintf(send_buffer, "%d", list[conn].port);
					send(sockfd, &send_buffer, sizeof(send_buffer), 0);
				
					pthread_create(&tid[0], NULL, keep_receiving, NULL);
					pthread_create(&tid[1], NULL, keep_sending, NULL);
				
					if(pthread_join(tid[0], (void**)&(ptr[0])) != 0)
						error("[x] ERROR : pthread_join 1 error\n", 0);
					if(pthread_join(tid[1], (void**)&(ptr[0])) != 0)
						error("[x] ERROR : pthread_join 2 error\n", 0);
				}else
					error("The client ID is wrong\n", 0);
	
			}break;
			case 3:{
				
				bzero(send_buffer, BUFFER_SZ);
				strcpy(send_buffer, "wait");
				send(sockfd, &send_buffer, sizeof(send_buffer), 0);
			
				bzero(rec_buffer, BUFFER_SZ);
				recv(sockfd, &rec_buffer, sizeof(rec_buffer), 0);
			
				conn = atoi(rec_buffer);
			
				bzero(send_buffer, BUFFER_SZ);
				strcpy(send_buffer, rec_buffer);
				printf("[.] Got a chat request\n");
				send(sockfd, &send_buffer, sizeof(send_buffer), 0);

				pthread_create(&tid[0], NULL, keep_receiving, NULL);
				pthread_create(&tid[1], NULL, keep_sending, NULL);
			
				if(pthread_join(tid[0], (void**)&(ptr[0])) != 0)
					error("[x] ERROR : pthread_join 1 error\n", 0);
				if(pthread_join(tid[1], (void**)&(ptr[0])) != 0)
					error("[x] ERROR : pthread_join 2 error\n", 0);

			}break;
			case 4:{
				bzero(send_buffer, BUFFER_SZ);
				strcpy(send_buffer, "logout");
				send(sockfd, &send_buffer, sizeof(send_buffer), 0);
				
				close(sockfd);
				flag = 0;
			
			}break;
			default:error("[x] ERROR : Wrong choice\n", 0);
		}
	}

	return 0;
}