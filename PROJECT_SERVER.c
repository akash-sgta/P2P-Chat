# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <pthread.h>
# include <unistd.h>
# include <errno.h>

# define BUFFER_SZ 1024

pthread_t tid[20];
int tind=0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct USER{
	int port;
	int socket;
	char ip[100];
	char name[100];
}user;
user list[100];

int ind;

char rec_buffer[BUFFER_SZ];
char send_buffer[BUFFER_SZ];

void error(char *str, int flag){
	perror(str);
	if(flag)
		exit(flag);
}

void test(char *arg, int flag, int line){
	char *f = (flag==1)?"rec_buffer":"send_buffer";
	printf("\n\n[-]\t%d\t%s\t-\t%s\n\n", line, f, arg);
}

int push_list(user *temp){
	
	pthread_mutex_lock(&clients_mutex);
	
	strcpy(list[ind].ip, temp->ip);
	strcpy(list[ind].name, temp->name);
	list[ind].port = temp->port;
	list[ind].socket = temp->socket;
	ind += 1;
			
	pthread_mutex_unlock(&clients_mutex);
	
	return 1;
	
}

int pop_list(user *rec_user, user *this_user){
	
	pthread_mutex_lock(&clients_mutex);
	
	int i, j;
	for(i=0; i<ind;i++){
		if(strcmp(rec_user->ip, list[i].ip) == 0 && rec_user->port == list[i].port){
			this_user->socket = list[i].socket;
			strcpy(this_user->ip, list[i].ip);
			this_user->port = list[i].port;
					
			for(j=i; j<ind; j++){
				strcpy(list[j].ip, list[j+1].ip);
				list[j].port = list[j+1].port;
				list[j].socket = list[j+1].socket;
				strcpy(list[j].name, list[j+1].name);
			}
			break;
		}
	}
	ind -= 1;
	
	pthread_mutex_unlock(&clients_mutex);
	
	return 1;
}

int show_list(int sockfd, user *temp){
	
	pthread_mutex_lock(&clients_mutex);
	
	int i;
	
	printf("[.] SERVER : AT : IP[%s] PORT[%d] NAME[%s]\n", temp->ip, temp->port, temp->name);
			
	bzero(send_buffer, BUFFER_SZ);
	sprintf(send_buffer, "%d", ind);
	send(sockfd, &send_buffer, sizeof(rec_buffer), 0);
	
	printf("[.] SERVER : LIST SIZE : [%d]\n", ind);
	printf("[.] The List : \n");

	for(i=0; i<ind; i++){
		printf("[.] IP[%s] PORT[%d] NAME[%s]\n", list[i].ip, list[i].port, list[i].name);
		
		if(strcmp(temp->ip, list[i].ip) == 0 && temp->port == list[i].port)
			continue;
		
		bzero(send_buffer, BUFFER_SZ);
		strcpy(send_buffer, list[i].ip);
		send(sockfd, &send_buffer, sizeof(send_buffer), 0);
		
		bzero(send_buffer, BUFFER_SZ);
		sprintf(send_buffer, "%d", list[i].port);
		send(sockfd, &send_buffer, sizeof(send_buffer), 0);

		bzero(send_buffer, BUFFER_SZ);
		strcpy(send_buffer, list[i].name);
		send(sockfd, &send_buffer, sizeof(send_buffer), 0);
	}
	
	pthread_mutex_unlock(&clients_mutex);
	
	return 1;
}

void *handler(void *socket){
	
	user new_user = *(user *)socket;
	int new_socket = new_user.socket;
	
	while(1){
		printf("[.] SERVER : WAITING : IP[%s] PORT[%d] NAME[%s]\n", new_user.ip, new_user.port, new_user.name);
		bzero(rec_buffer, BUFFER_SZ);
		recv(new_socket, &rec_buffer, sizeof(rec_buffer), 0);
		
		printf("[.] SERVER : PROCESSING REQUEST FROM : IP[%s] PORT[%d] NAME[%s]\n", new_user.ip, new_user.port, new_user.name);
		printf("[.] >>> %s\n", rec_buffer);

		if(strncmp("request", rec_buffer, 7) == 0){
			
			if(show_list(new_socket, &new_user))
				printf("[.] SUCCESSFULLY SHOWN LIST\n");
			
		}else if(strncmp("connect", rec_buffer,  7) == 0){
			
			printf("[.] SERVER : AT : IP[%s] PORT[%d] NAME[%s]\n", new_user.ip, new_user.port, new_user.name);
			user *rec_user;
			
			rec_user = (user*)malloc(sizeof(user));
			
			bzero(rec_buffer, BUFFER_SZ);
			recv(new_socket, &rec_buffer, sizeof(rec_buffer), 0);
			
			strcpy(rec_user->ip, rec_buffer);
			
			bzero(rec_buffer, BUFFER_SZ);
			recv(new_socket, &rec_buffer, sizeof(rec_buffer), 0);
			
			rec_user->port = atoi(rec_buffer);

			user *this_user;
			
			this_user = (user*)malloc(sizeof(user));
			
			if(pop_list(rec_user, this_user))
				printf("[.] SUCCESSFULLY POPPED FROM LIST\n");

			bzero(send_buffer, BUFFER_SZ);
			sprintf(send_buffer, "%d", new_socket);
			send(this_user->socket, &send_buffer, sizeof(send_buffer), 0);

			printf("[.] SERVER : AT : IP[%s] PORT[%d] NAME[%s]\n", new_user.ip, new_user.port, new_user.name);
			printf("[.] SERVER : STARTING CHAT AT CONNECT SIDE.....\n");

			while(1){
				
				bzero(rec_buffer, BUFFER_SZ);
				recv(new_socket, &rec_buffer, sizeof(rec_buffer), 0);
				
				if(strstr(rec_buffer, ":die:") != 0){
					break;
				}
				
				bzero(send_buffer, BUFFER_SZ);
				strcpy(send_buffer, rec_buffer);
				send(this_user->socket, &send_buffer, sizeof(send_buffer), 0);
				
				if(strstr(rec_buffer, ":end:") != 0){
					break;
				}

			}

			printf("[.] SERVER : AT : IP[%s] PORT[%d]\n", new_user.ip, new_user.port);
			printf("[.] SERVER : CLOSING CHAT FROM CONNECT SIDE\n");
			
			free(this_user);
			free(rec_user);
		
		}else if(strncmp("wait", rec_buffer, 4) == 0){
			
			printf("[.] SERVER : AT : IP[%s] PORT[%d] NAME[%s]\n", new_user.ip, new_user.port, new_user.name);
			printf("[.] SERVER : ADDING TO WAIT LIST\n");
			
			if(push_list(&new_user))
				printf("[.] SUCCESSFULLY PUSHED TO LIST\n");

			int this_socket;
			
			printf("[.] SERVER : WAITING FOR CHAT REQUEST...\n");
			
			bzero(rec_buffer, BUFFER_SZ);
			recv(new_socket, &rec_buffer, sizeof(rec_buffer), 0);
						
			this_socket = atoi(rec_buffer);
			
			printf("[.] SERVER : AT : IP[%s] PORT[%d]\n", new_user.ip, new_user.port);
			printf("[.] SERVER : RECIEVED CHAT REQUEST\n");

			while(1){
				
				bzero(rec_buffer, BUFFER_SZ);
				recv(new_socket, &rec_buffer, sizeof(rec_buffer), 0);
				
				if(strstr(rec_buffer, ":die:") != 0){
					break;
				}

				bzero(send_buffer, BUFFER_SZ);
				strcpy(send_buffer, rec_buffer);
				send(this_socket, &send_buffer, sizeof(send_buffer), 0);
				
				if(strstr(rec_buffer, ":end:") != 0){
					break;
				}
			}

			printf("[.] SERVER : AT : IP[%s] PORT[%d]\n", new_user.ip, new_user.port);
			printf("[.] SERVER : CLOSED AT WAIT SIDE\n");
			
			
			
		}else if(strncmp("logout", rec_buffer, 6) == 0){
			
			printf("[.] SERVER : AT : IP[%s] PORT[%d]\n", new_user.ip, new_user.port);
			printf("[.] SERVER : CLOSING CONNECTION\n");
			close(new_socket);
			break;
			
		}else{
			
			printf("[.] SERVER : WRONG INPUT\n");
			printf("[.] SERVER : AT : IP[%s] PORT[%d]\n", new_user.ip, new_user.port);
			printf("[.] SERVER : CLOSING CONNECTION\n");
			close(new_socket);
			break;
		}
	}
	pthread_exit(0);
}

int main(int argc, char *argv[]){
	
	if(argc < 2)
		error("[x] ERROR : USAGE -> filename port_number\n",1);
	
	struct sockaddr_in client, server;
	int cli_len, sockfd;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server.sin_family = AF_INET;
	server.sin_port = atoi(argv[1]);
	server.sin_addr.s_addr = INADDR_ANY;
	
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
		error("[x] ERROR : setsockopt(SO_REUSEADDR | SO_REUSEPORT)\n", 1);

	if(bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
		error("[x] ERROR : Binding failed\n", 1);

	system("clear");
	printf("\n");
	printf("****************************************\n");
	printf("*                                      *\n");
	printf("*     WELCOME TO PEER TO PEER CHAT     *\n");
	printf("*           Language used : C          *\n");
	printf("*     For Operating System : Linux     *\n");
	printf("*       Author : Akash Sengupta        *\n");
	printf("*     Github repository : nibbax64     *\n");
	printf("*             ---SERVER---             *\n");
	printf("*                                      *\n");
	printf("****************************************\n\n");
	
	printf("[.] SERVER STARTED SUCCESSFULLY...\n");
	printf("\n** PLEASE DO NOT USE CTRL+C OR ANY OTHER SYSTEM EXIT COMMAND UNLESS EXTREMELY NECESSARY **\n");

	cli_len = sizeof(client);

	if(listen(sockfd, 2) < 0)
		error("[x] ERROR : LISTENING TO PORTS\n", 1);
	else
		printf("[.] LISTENING.....\n");

	user obj;
	
	while(1){
		
		int new_socket = accept(sockfd, (struct sockaddr *)&client, &cli_len);
		printf("[.] SERVER : NEW CONNECTION --> IP[%s] PORT[%d] ", inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		
		obj.socket = new_socket;
		obj.port = ntohs(client.sin_port);
		strcpy(obj.ip, inet_ntoa(client.sin_addr));
		
		bzero(rec_buffer, BUFFER_SZ);
		recv(new_socket, &rec_buffer, sizeof(rec_buffer), 0);
		strcpy(obj.name, rec_buffer);
		
		printf("NAME[%s]\n", rec_buffer);
		
		if(pthread_create(&(tid[tind]), NULL, handler, (void *)&obj) != 0)
			error("[T] ERROR : pthread_create() error\n", 1);

		tind += 1;
		
		if(pthread_join(tid[tind], NULL) == 0)
			error("[T] ERROR : pthread_join()\n", 1);
	}

	close(sockfd);
	return 0;
}