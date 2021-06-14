#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_EVENTS 1024  /* Maximum number of events to process*/
#define LEN_NAME 16  /* Assuming that the length of the filename
won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME ))
/*buffer to store the data of events*/

void sig_handler_parent(int signum){
  printf("\n");
}

void sig_handler_child(int signum){
    printf("OK\n");
  sleep(1);
  kill(getppid(),SIGUSR1);
}

int watch(){
    int fd,wd;
    int i=0;
    char file[] = "test.x";
    
    char buf[BUF_LEN];
    /* Step 1. Initialize inotify */
       fd = inotify_init();
       if ( fd < 0 ) {
           perror( "Couldn't initialize inotify");
       }
       /* Step 2. Add Watch */
       wd = inotify_add_watch(fd,"/mnt/c/Users/Great/Documents/treinoWSL/progC/topicosC/topicos_comp/atv_1", IN_CREATE );
 
       if(wd==-1){
               printf("Could not watch : \n");
       }
       else{
              printf("Watching : %s\n","/mnt/c/Users/Great/Documents/treinoWSL/progC/topicosC/topicos_comp/atv_1");
       }
 
 
       while(1){
           int length  = read(fd,buf,BUF_LEN );
           while ( i < length ) {
                struct inotify_event *event = ( struct inotify_event * ) &buf[ i ];
                if ( event->len ) {
                    if ( event->mask & IN_CREATE) {
                        if (strcmp(file, event->name) == 0){
                            printf( "The file %s was Created with WD %d\n", event->name, event->wd ); 
                            return 0;
                        }
                                  
                    }
                
                    i += EVENT_SIZE + event->len;
                }    
            }
        }
             
    
    inotify_rm_watch( fd, wd );
    close( fd );
}

int main(){
    
    pid_t pid = fork();

    if(pid == 0){ // processo B
        signal(SIGUSR1,sig_handler_child);
        pause();
    }
    else{ // processo A
       
       signal(SIGUSR1,sig_handler_parent); // Register signal handler
       watch();
       sleep(1);
       kill(pid,SIGUSR1);  
        pause();
    }
    
    return 0;
}