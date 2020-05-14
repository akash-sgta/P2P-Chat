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

# define BUFFER_SZ 1024

pthread_t tid[50];
int tind=0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct USER{
	int port;
	int socket;
	char ip[100];
	char name[100];
}user;
user list [100];

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

void *handler(void *socket){
	
	user new_user = *(user *)socket;
	int new_socket = new_user.socket;
	int i, j;

	while(1){
		printf("[.] SERVER : WAITING : IP[%s] PORT[%d] NAME[%s]\n", new_user.ip, new_user.port, new_user.name);
		bzero(rec_buffer, BUFFER_SZ);
		recv(new_socket, &rec_buffer, sizeof(rec_buffer), 0);
		
		// test(rec_buffer, 1, 50);

		printf("[.] SERVER : PROCESSING REQUEST FROM : IP[%s] PORT[%d] NAME[%s]\n", new_user.ip, new_user.port, new_user.name);
		printf("[.] >>> %s\n", rec_buffer);

		if(strncmp("request", rec_buffer, 7) == 0){
			
			printf("[.] SERVER : AT : IP[%s] PORT[%d] NAME[%s]\n", new_user.ip, new_user.port, new_user.name);
			
			bzero(send_buffer, BUFFER_SZ);
			sprintf(send_buffer, "%d", ind);
			
			// test(send_buffer, 1, 66);
			
			send(new_socket, &send_buffer, sizeof(rec_buffer), 0);
			
			printf("[.] SERVER : LIST SIZE : [%d]\n", ind);

			printf("[.] The List : \n");

			for(i=0; i<ind; i++){
				printf("[.] IP[%s] PORT[%d]\n", list[i].ip, list[i].port);
				if(strcmp(new_user.ip, list[i].ip) == 0 && new_user.port == list[i].port)
					continue;
				
				bzero(send_buffer, BUFFER_SZ);
				strcpy(send_buffer, list[i].ip);
				
				// test(send_buffer, 2, 82);
				
				send(new_socket, &send_buffer, sizeof(send_buffer), 0);
				
				bzero(send_buffer, BUFFER_SZ);
				sprintf(send_buffer, "%d", list[i].port);
				
				// test(send_buffer, 2, 89);
				
				send(new_socket, &send_buffer, sizeof(send_buffer), 0);
				
				bzero(send_buffer, BUFFER_SZ);
				strcpy(send_buffer, list[i].name);
				
				// test(send_buffer, 2, 92);
				
				send(new_socket, &send_buffer, sizeof(send_buffer), 0);
			}
			
		}else if(strncmp("connect", rec_buffer,  7) == 0){
			
			printf("[.] SERVER : AT : IP[%s] PORT[%d] NAME[%s]\n", new_user.ip, new_user.port, new_user.name);
			char rec_ip[100];
			int rec_port;
			
			bzero(rec_buffer, BUFFER_SZ);
			recv(new_socket, &rec_buffer, sizeof(rec_buffer), 0);
			
			// test(rec_buffer, 1, 99);
			
			strcpy(rec_ip, rec_buffer);
			
			bzero(rec_buffer, BUFFER_SZ);
			recv(new_socket, &rec_buffer, sizeof(rec_buffer), 0);
			
			// test(rec_buffer, 1, 106);
			
			rec_port = atoi(rec_buffer);

			char this_ip[100]; 
			int this_socket, this_port;

			for(i=0; i<ind;i++){
				if(strcmp(rec_ip, list[i].ip) == 0 && rec_port == list[i].port){
					this_socket = list[i].socket;
					strcpy(this_ip, list[i].ip);
					this_port = list[i].port;
					for(j=i; j<ind; j++){
						strcpy(list[j].ip, list[j+1].ip);
						list[j].port = list[j+1].port;
						list[j].socket = list[j+1].socket;
					}
					break;
				}
			}
			ind -= 1;

			bzero(send_buffer, BUFFER_SZ);
			sprintf(send_buffer, "%d", new_socket);
			
			// test(send_buffer, 2, 135);
			
			send(this_socket, &send_buffer, sizeof(send_buffer), 0);

			printf("[.] SERVER : AT : IP[%s] PORT[%d]\n", new_user.ip, new_user.port);
			printf("[.] SERVER : STARTING CHAT AT CONNECT SIDE\n");

			while(1){
				
				bzero(rec_buffer, BUFFER_SZ);
				recv(new_socket, &rec_buffer, sizeof(rec_buffer), 0);
				
				// test(rec_buffer, 1, 143);
				

				if(strncmp("die", rec_buffer, 3) == 0)
					break;
				
				bzero(send_buffer, BUFFER_SZ);
				strcpy(send_buffer, rec_buffer);
				
				// test(send_buffer, 2, 156);
				
				send(this_socket, &send_buffer, sizeof(send_buffer), 0);
				if(strcmp(rec_buffer,"end")==0)
					break;
			}

			printf("[.] SERVER : AT : IP[%s] PORT[%d]\n", new_user.ip, new_user.port);
			printf("[.] SERVER : CLOSING CHAT FROM CONNECT SIDE\n");

		
		}else if(strncmp("wait", rec_buffer, 4) == 0){
			
			printf("[.] SERVER : AT : IP[%s] PORT[%d] NAME[%s]\n", new_user.ip, new_user.port, new_user.name);

			strcpy(list[ind].ip, new_user.ip);
			strcpy(list[ind].name, new_user.name);
			list[ind].port = new_user.port;
			list[ind].socket = new_user.socket;
			ind += 1;

			char this_ip;
			int this_port, this_socket;
			printf("[.] SERVER : WAITING FOR CHAT REQUEST\n");
			
			bzero(rec_buffer, BUFFER_SZ);
			recv(new_socket, &rec_buffer, sizeof(rec_buffer), 0);
			
			// test(rec_buffer, 1, 180);
			
			this_socket = atoi(rec_buffer);
			
			printf("[.] SERVER : AT : IP[%s] PORT[%d]\n", new_user.ip, new_user.port);
			printf("[.] SERVER : RECIEVED CHAT REQUEST\n");

			while(1){
				
				bzero(rec_buffer, BUFFER_SZ);
				recv(new_socket, &rec_buffer, sizeof(rec_buffer), 0);
				
				// test(rec_buffer, 1, 192);
				

				if(strncmp("die", rec_buffer, 3) == 0)
					break;

				bzero(send_buffer, BUFFER_SZ);
				strcpy(send_buffer, rec_buffer);
				
				// test(send_buffer, 2, 205);
				
				send(this_socket, &send_buffer, sizeof(send_buffer), 0);
				
				if(strncmp("end", rec_buffer, 3) == 0)
					break;
			}

			printf("[.] SERVER : AT : IP[%s] PORT[%d]\n", new_user.ip, new_user.port);
			printf("[.] SERVER : CLOSED AT WAIT SIDE\n");
			
		}else if(strncmp("logout", rec_buffer, 6) == 0){
			
			printf("[.] SERVER : AT : IP[%s] PORT[%d]\n", new_user.ip, new_user.port);
			printf("[.] SERVER : CLOSING CONNECTION\n");
			printf("[.] SERVER : REMOVING FROM LIST\n");
			close(new_socket);
			return 0;
			
		}else{
			
			printf("[.] SERVER : WRONG INPUT\n");
			printf("[.] SERVER : AT : IP[%s] PORT[%d]\n", new_user.ip, new_user.port);
			printf("[.] SERVER : CLOSING CONNECTION\n");
			printf("[.] SERVER : REMOVING FROM LIST\n");
			close(new_socket);
			return 0;
		}
	}
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

	if(bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
		error("[x] ERROR : Binding failed\n", 1);

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
		
		pthread_create(&(tid[tind]), NULL, handler, (void *)&obj);
		
		tind += 1;
	}

	close(sockfd);
	return 0;
}
