#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
//#include <errno.h>
static _Atomic unsigned int cli_count = 0;
#define LOCAL_HOST "127.0.0.1"
#define BUFFER 1024
#define MAX_CLIENTES 2
static int uid = 10;

pthread_mutex_t lock;

typedef struct{
    int flag;
	int socket;
	int uid;
	char name[32];
} client_t;

client_t *clients[50];

void remove_extra_caracteres(char * str){
    char *ch;                       
    char c;    
    ch = str;
    while (*ch != '\n' &&  *ch != '\0') {
            ++ch;
        }
        if (*ch) {
            *ch = '\0';
        } else {         // remove any extra characters in input stream
            while ((c = getchar()) != '\n' && c != EOF)
                continue;
        }
}
void send_message(char *s, int uid){
	pthread_mutex_lock(&lock);

	for(int i=0; i<MAX_CLIENTES; ++i){
		if(clients[i]){
			if(clients[i]->uid != uid){
				if(write(clients[i]->socket, s, strlen(s)) < 0){
					perror("ERROR: write to descriptor failed");
					break;
				}
			}
		}
	}

	pthread_mutex_unlock(&lock);
}
void *fila(void *arg){
    printf("fila");
    fflush(stdout);
    cli_count ++;
    int flag = 0;
	char msgBuff[2048];
    char msgName[32];
    char * message;
    client_t *cliente = (client_t *)arg;
    //printf("receber certo");
    fflush(stdout);
    //recv(cliente->socket, msgName, 32,0);
    recv(cliente->socket, msgBuff, 2048,0);
    remove_extra_caracteres(msgBuff);
    printf(msgBuff);
    //enviar_msg_clientes(msgBuff,cliente->socket);
    
    strcpy(cliente->name,msgName);
    
    printf("%s é o numero %d da fila", cliente->name,cli_count);
    fflush(stdout);
}

void *receber(void *arg)
{
    pthread_mutex_lock(&lock); 
    //cli_count ++;
    int flag = 0;
	char msgBuff[2048];
    char msgName[32];
    char * message;
    client_t *cliente = (client_t *)arg;

    recv(cliente->socket, msgName, 32, 0);
    strcpy(cliente->name, msgName);
    printf("\n%s entrou na sala",msgName);
    fflush(stdout);
	while(1){
		if (flag == 1) {
			break;
		}

		int msg_recv = recv(cliente->socket, msgBuff, 2048, 0);
		if (msg_recv>0){
			//printf("%s", msgBuff);
			send_message(msgBuff, cliente->uid);
            remove_extra_caracteres(msgBuff);
            printf("%s -> %s\n", msgBuff, cliente->name);
			
		}
        if ( strcmp(msgBuff, "/ENTRAR") == 0){
            message = "Seja bem vindo";
            write(cliente->socket,message, 2048);
        } else {
			printf("ERROR: -1\n");
            fflush(stdout);
			flag = 1;
		}
        
        fflush(stdout);
		bzero(msgBuff, 2048);
	}
	pthread_mutex_unlock(&lock); 
    pthread_exit(NULL);  
}	

int main(){
    
    int sockfd, new_socket, valread;  
    int opt = 1;
    char msg[BUFFER];
    struct sockaddr_in endLoc;
    int addrlen = sizeof(endLoc);
    //struct sockaddr_in client_addr;
    pthread_t tid;
    
    
    if ( (sockfd = socket(AF_INET , SOCK_STREAM, 0) ) == 0){
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    memset(&endLoc,'\0', sizeof(endLoc));
    endLoc.sin_family = AF_INET;
	endLoc.sin_addr.s_addr = inet_addr("127.0.0.1");
	endLoc.sin_port = htons(9999);

    if(bind(sockfd, (struct sockaddr*)&endLoc, sizeof(endLoc)) < 0 ){
        perror("error na função bind()");
        exit(EXIT_FAILURE);
    }
    
    if(listen(sockfd , 1) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //printf("Bem vindo");
    fflush(stdout);
    while(1){
        
        if ((new_socket = accept(sockfd, (struct sockaddr *)&endLoc, 
                       (socklen_t*) &addrlen  ))<0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
         if((cli_count + 1) == MAX_CLIENTES){
			printf("Sem vaga para mais clientes. Recusado: ");
			close(new_socket);
			continue;
		}
        client_t *cli;
        cli->flag = 0;
        cli->socket = new_socket;
		cli->uid = uid++;
        
        
        //pthread_create(&tid[0], NULL, &fila, (void*)cli);
        //pthread_join(tid[0],NULL);
        pthread_create(&tid, NULL, &receber, (void*)cli);
        //pthread_join(tid,NULL);
        sleep(1);
       // fila(new_socket);
       

    }
    
    return EXIT_SUCCESS;
}
