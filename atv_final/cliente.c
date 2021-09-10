#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LENGTH 2048

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];

void remove_extra_caracteres(char * str){
    char *ch;                       // used to remove newline
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

void send_msg() {
  char message[LENGTH] = {};
  char buffer[LENGTH + 32] = {};
   
  while(1) {
  	printf("%s", "> ");
    fflush(stdout);
    fgets(message, LENGTH, stdin);
    remove_extra_caracteres(message);
    
    if (strcmp(message, "/SAIR") == 0) {
            flag = 1;
			break;
    } else {
      sprintf(buffer, "%s:%s", name, message);
      send(sockfd, buffer, strlen(buffer), 0);
    }

	bzero(message, LENGTH);
    bzero(buffer, LENGTH + 32);
  }
  //catch_ctrl_c_and_exit(2);
}

void receive() {
	char message[LENGTH] = {};
    fflush(stdout);
    while (1) {
		int receive = recv(sockfd, message, LENGTH, 0);
        if (receive > 0) {
        printf("%s", message);
        printf("%s", "> ");
        fflush(stdout);
        } else if (receive == 0) {
                break;
        } else {
			// -1
		}
		memset(message, 0, sizeof(message));
    }
}

int main(int argc, char **argv){
	
	char *ip = "127.0.0.1";
    strcpy(name,argv[1]);
	if (strlen(name) > 32 || strlen(name) < 2){
		printf("O nome precisa ter menos de 30 caracteres e mais que 2.\n");
		return EXIT_FAILURE;
	}
    
	struct sockaddr_in server_addr;

	/* Definições do Socket  */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(9999);

    if(inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    
    // Conexão com o servidor
    int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (err == -1) {
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}

	// Envia o nome 
	send(sockfd, name, 32, 0);

	printf("--- Caso queira sair digite: /SAIR)---\n");
    

	pthread_t send_msg_thread;
    if(pthread_create(&send_msg_thread, NULL, (void *) send_msg, NULL) != 0){
		printf("ERROR: pthread\n");
        return EXIT_FAILURE;
	}
    

	pthread_t recv_msg_thread;
    if(pthread_create(&recv_msg_thread, NULL, (void *) receive, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1){
		if(flag){
			printf("\nSaindo...\n");
			break;
        }
	}

	close(sockfd);

	return EXIT_SUCCESS;
}