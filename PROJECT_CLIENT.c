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

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

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
		
		// test(rec_buffer, 1, 46);
		
		if(strlen(rec_buffer) == 0)
			continue;

		bzero(rec_buffer, BUFFER_SZ);
		recv(sockfd, &rec_buffer, sizeof(rec_buffer), 0);
		
		// test(rec_buffer, 1, 54);
		

		if(strstr(send_buffer, ":end:") != 0){
			
			bzero(send_buffer, BUFFER_SZ);
			strcpy(send_buffer, "die");
			
			// test(send_buffer, 2, 66);
			
			send(sockfd, &send_buffer, sizeof(send_buffer), 0);

			if(pthread_cancel(tid[1]) == 0)
				printf("[.] keep_receiving  -(cancelled)->  keep_sending\n");
			else
				printf("[.] keep_receiving failed\n");

			int ret = 0;
			pthread_exit(&ret);
		}
		printf("%s\n", rec_buffer);
	}

}

void *keep_sending(){
	
	char buffer[BUFFER_SZ];
	int i;
	
	while(1){

		bzero(send_buffer, BUFFER_SZ);
		scanf("%[^\n]", buffer);
		getchar();
		
		if(strlen(buffer) == 0)
			continue;
		
		for(i=0; i<BUFFER_SZ; i++){
			if(buffer[i] == ' ')
				buffer[i] = '_';
			if(buffer[i] == '\n' || buffer[i] == '\0'){
				buffer[i] = '\0';
				break;
			}
			
		}
		
		sprintf(send_buffer, "[%s] -> { %s }", name, buffer);
		
		// test(send_buffer, 2, 102);
		
		send(sockfd, &send_buffer, sizeof(send_buffer), 0);

		if(strstr(send_buffer, ":end:") != 0){
			if(pthread_cancel(tid[0]) == 0)
				printf("[.] keep_sending  -(cancelled)-> keep_receiving\n");
			else
				printf("[.] keep_sending failed\n");

			int ret = 0;
			pthread_exit(&ret);
		}
	}
}

void update_list(){
	
	bzero(send_buffer, BUFFER_SZ);
	strcpy(send_buffer, "request");
	
	// test(send_buffer, 2, 111);
	
	send(sockfd, &send_buffer, sizeof(send_buffer), 0);
	
	int num;
	
	bzero(rec_buffer, BUFFER_SZ);
	recv(sockfd, &rec_buffer, sizeof(rec_buffer), 0);
	
	// test(rec_buffer, 1, 116);
	
	num = atoi(rec_buffer);
	ind = 0;
	
	printf("\n----------------------------------------\n");
	printf("[.]\t[ID]\t:\t[IP]\t:\t[PORT]\t:\t[NAME]\n");
	while(num--){
		
		char ip[100], name[100];
		int port;
		
		bzero(rec_buffer, BUFFER_SZ);
		recv(sockfd, &rec_buffer, sizeof(rec_buffer), 0);
		
		// test(rec_buffer, 1, 130);
		
		strcpy(ip, rec_buffer);
		
		bzero(rec_buffer, BUFFER_SZ);
		recv(sockfd, &rec_buffer, sizeof(rec_buffer), 0);
		
		// test(rec_buffer, 1, 137);
		
		port = atoi(rec_buffer);
		
		bzero(rec_buffer, BUFFER_SZ);
		recv(sockfd, &rec_buffer, sizeof(rec_buffer), 0);
		
		// test(rec_buffer, 1, 130);
		
		strcpy(name, rec_buffer);
		
		strcpy(list[ind].ip, ip);
		strcpy(list[ind].name, name);
		list[ind].port = port;
		printf("[.]\t[%d]\t:\t[%s]\t:\t[%d]\t:\t[%s]\n", ind, ip, port, name);
		ind += 1;
	}
	printf("\n----------------------------------------\n");
}

int main(int argc, char *argv[]){
	
	if(argc < 3)
		error("[x] ERROR : USAGE -> filename ip_address port_number\n", 1);
	
	struct sockaddr_in client,server;
	int flag = 1;

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	server.sin_family = AF_INET;
	server.sin_port = atoi(argv[2]);
	server.sin_addr.s_addr=inet_addr(argv[1]);
	
	printf("[.] Client side has been setup successfully...\n");

	n = sizeof(server);

	if(connect(sockfd, (struct sockaddr *)&server, n) < 0)
		error("[x] ERROR : Connection failure\n", 1);

	bzero(send_buffer, BUFFER_SZ);
	printf("[.] USER_NAME : ");
	scanf("%s", name);
	strcpy(send_buffer, name);
	send(sockfd, &send_buffer, sizeof(send_buffer), 0);
	
	while(flag){
		
		printf("\n");
		printf("[1] Get the list of clients\n");
		printf("[2] Connect to a client\n");
		printf("[3] Wait for connection\n");
		printf("[4] Logout\n\n[.] > ");
		
		int dec;
		scanf("%d", &dec);

		switch(dec){
			case 1:{
				update_list();
			}break;
			case 2:{
				update_list();
			
				bzero(send_buffer, BUFFER_SZ);
				strcpy(send_buffer, "connect");
			
				// test(send_buffer, 2, 190);
			
				send(sockfd, &send_buffer, sizeof(send_buffer), 0);
				printf("[.] Enter the id of client you want to connect to : ");

				int conn;
				scanf("%d", &conn);

				if(conn >= ind){
					error("The client ID is wrong\n", 0);
					continue;
				}

				bzero(send_buffer, BUFFER_SZ);
				strcpy(send_buffer, list[conn].ip);
				
				// test(send_buffer, 2, 206);
				
				send(sockfd, &send_buffer, sizeof(send_buffer), 0);
				
				bzero(send_buffer, BUFFER_SZ);
				sprintf(send_buffer, "%d", list[conn].port);
				
				// test(send_buffer, 2, 213);
				
				send(sockfd, &send_buffer, sizeof(send_buffer), 0);
				
				pthread_create(&tid[0], NULL, keep_receiving, NULL);
				pthread_create(&tid[1], NULL, keep_sending, NULL);
				
				if(pthread_join(tid[0], (void**)&(ptr[0])) != 0)
					error("[x] ERROR : pthread_join 1 error\n", 0);
				if(pthread_join(tid[1], (void**)&(ptr[0])) != 0)
					error("[x] ERROR : pthread_join 2 error\n", 0);
	
			}break;
			case 3:{
				int conn;

				bzero(send_buffer, BUFFER_SZ);
				strcpy(send_buffer, "wait");
			
				// test(send_buffer, 2, 232);
			
				send(sockfd, &send_buffer, sizeof(send_buffer), 0);
			
				bzero(rec_buffer, BUFFER_SZ);
				recv(sockfd, &rec_buffer, sizeof(rec_buffer), 0);
			
				// test(rec_buffer, 1, 235);
			
				conn = atoi(rec_buffer);
			
				bzero(send_buffer, BUFFER_SZ);
				strcpy(send_buffer, rec_buffer);
				printf("[.] Got a chat request\n");
			
				// test(send_buffer, 2, 247);
			
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
			
				// test(send_buffer, 2, 264);
			
				send(sockfd, &send_buffer, sizeof(send_buffer), 0);
				close(sockfd);
				flag = 0;
			
			}break;
			default:error("[x] ERROR : Wrong choice\n", 0);
		}
	}

	return 0;
}