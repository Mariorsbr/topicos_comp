#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <signal.h>

#define MAX_CLIENTS 4 //permite apenas 3 clientes
#define BUFFER_SZ 2048

static _Atomic unsigned int cli_count = 0;
static int uid = 10;

/* Client structure */
typedef struct{
	struct sockaddr_in address;
	int sockfd;
	int uid;
	char name[32];
} client_t;

client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void send_message(char *s, int uid){
	pthread_mutex_lock(&clients_mutex);

	for(int i=0; i<MAX_CLIENTS; ++i){
		if(clients[i]){
			if(clients[i]->uid != uid){
				if(write(clients[i]->sockfd, s, strlen(s)) < 0){
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&clients_mutex);
}

void *communication(void *arg){
	char buff_out[BUFFER_SZ];
	char name[32];
	int leave_flag = 0;

	cli_count++;
	client_t *cli = (client_t *)arg;
	
    if(recv(cli->sockfd, name, 32, 0)>0){
        strcpy(cli->name, name);
		sprintf(buff_out, "%s entrou na sala\n", cli->name);
		printf("%s", buff_out);
    }
	
	while(1){
		if (leave_flag) {
			break;
		}

		int receive = recv(cli->sockfd, buff_out, BUFFER_SZ, 0);
		if (receive > 0){
			if(strlen(buff_out) > 0){
				send_message(buff_out, cli->uid);
                for (int i = 0; i<BUFFER_SZ; i++){
                     if( buff_out[i] == '\n'){
                         buff_out[i] = '\0';
                     }
                }
				printf("%s -> %s\n", buff_out, cli->name);
			}
		} else if (receive == 0 || strcmp(buff_out, "/SAIR") == 0){
			sprintf(buff_out, "%s saiu\n", cli->name);
			printf("%s", buff_out);
			send_message(buff_out, cli->uid);
			leave_flag = 1;
		} else {
			printf("ERROR: -1\n");
			leave_flag = 1;
		}

		bzero(buff_out, BUFFER_SZ);
	}
	close(cli->sockfd);
    pthread_mutex_lock(&clients_mutex);
    cli = NULL;
    free(cli);
    cli_count--;
    pthread_mutex_unlock(&clients_mutex);
    pthread_detach(pthread_self());

	return NULL;
}

int main(){

	char *ip = "127.0.0.1";
	//int port = atoi(argv[1]);
	int option = 1;
	int listenfd = 0, connfd = 0;

    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    pthread_t tid;

    /* Definições do Socket  -- AF_INET = ipV4, SOCK_STREAM = TCP */ 
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(9999);

    /* Ignore pipe signals */
	signal(SIGPIPE, SIG_IGN);

	if(setsockopt(listenfd, SOL_SOCKET,(SO_REUSEPORT | SO_REUSEADDR),(char*)&option,sizeof(option)) < 0){
		perror("ERROR: setsockopt failed");
    return EXIT_FAILURE;
	}

	/* Bind */
    if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR: Socket binding failed");
        return EXIT_FAILURE;
    }

    /* Listen */
    if (listen(listenfd, 10) < 0) {
    perror("ERROR: Socket listening failed");
    return EXIT_FAILURE;
	}

	while(1){
		socklen_t clilen = sizeof(cli_addr);
		connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen);
        if(connfd<0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
		/* Checagem do numero de clientes*/
        pthread_mutex_lock(&clients_mutex);
		if((cli_count + 1) == MAX_CLIENTS){
			printf("Sem vaga para mais clientes. Recusado: ");
			close(connfd);
			continue;
		}
        pthread_mutex_unlock(&clients_mutex);
		/* Client settings */
		client_t *cli = (client_t *)malloc(sizeof(client_t));
		cli->address = cli_addr;
		cli->sockfd = connfd;
		cli->uid = uid++;

        for(int i=0; i<MAX_CLIENTS; i++){
            if(!clients[i] ){
                clients[i]= cli;
                break;
            }
        }
		pthread_create(&tid, NULL, &communication, (void*)cli);
		/* Reduce CPU usage */
		sleep(1);
	}

	return EXIT_SUCCESS;
}