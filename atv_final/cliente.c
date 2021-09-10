#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
//#include <errno.h>
#define LOCAL_HOST "127.0.0.1"
#define LENGTH 2048
char name[32];
int sock = 0;
volatile sig_atomic_t flag = 0;

pthread_mutex_t lock;

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

void enviar_msg(){
    char message[LENGTH] = {};
    char buffer[LENGTH + 32] = {};         
    while(1) {
        printf("%s", "> ");
        fgets(message, LENGTH, stdin);
        remove_extra_caracteres(message);
        fflush(stdout);
        continue;
        if ( strcmp(message,"/SAIR") == 0) {
                break;
        }
        else {
            sprintf(buffer, "%s:%s", name, message);   
            send(sock, buffer, strlen(buffer), 0);
        }
        bzero(message, LENGTH);
        bzero(buffer, LENGTH + 32);
   }

   flag = 1;
}

void receber_msg(){
    char message[LENGTH] = {};
    //printf("receber aqui");
    fflush(stdout);
    while (1) {
        if (recv(sock, message, LENGTH, 0)> 0) {
            printf("%s", "> ");
            fflush(stdout);
        } else {
                break;
        }
		memset(message, 0, sizeof(message));
    }
}

int main(int argc, char **argv){
    
    strcpy(name, argv[1]);
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    //char *hello = "Hello from client";
    char buffer[1024] = {0};
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
   
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9999);
       
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    
   
    send(sock,name,32,0);
    
    printf("Caso queria sair digite: /SAIR  \n");
    fflush(stdout);

    pthread_t enviar_msg_th;
    if(pthread_create(&enviar_msg_th, NULL, (void *) enviar_msg,NULL) != 0){
		printf("ERROR: pthread\n");
        return EXIT_FAILURE;
	}
    
    
    pthread_t receber_msg_th;
    if(pthread_create(&receber_msg_th, NULL, (void *) receber_msg, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}
    
    while(1){
        if(flag){
            printf("saindo...");
            break;
        }
    }
    
    close(sock);

    return EXIT_SUCCESS;

}