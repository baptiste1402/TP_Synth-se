#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>


#define BUFFSIZE 1096
void exeCommand(char** buf,ssize_t commande_size);
char ** SeparateCommande (char* commande);

int main(){
    char welcome[BUFFSIZE] = "Bienvenue dans le shell de l'ensea.\nPour quitter tapez 'exit'.\n";
    char buf[BUFFSIZE];
    char ** liste_commande;
    ssize_t commande_size;
    write(STDOUT_FILENO,welcome,strlen(welcome));
    memset(buf,0,BUFFSIZE); // filled buf with zero to avoid errors
       while(1){ // infinite loop that handles the commands
        write(1,"enseash %",strlen("enseash %"));
        commande_size = read(0,buf,BUFFSIZE); // allows you to know the number of characters obtained
        liste_commande = SeparateCommande(buf);
        exeCommand(liste_commande,commande_size);
   }

}

void exeCommand(char** buf,ssize_t commande_size)
{
    int pid, status;
    struct timespec start,stop; 
    int time_execution_ns, time_execution_s, time_execution_ms;
    int fd_in = -1, fd_out = -1;
    char ** new_buf= (char**)malloc(20* sizeof(char*));
    int j =0;
    
    if(strcmp("exit",buf[0])==0 || commande_size ==0){
        write(1,"en revoir",strlen("en revoir"));
        exit(EXIT_FAILURE);}
    for(int i = 0; buf[i]!= NULL; i++){
        if(strcmp(buf[i],"<")==0){
            fd_in = open(buf[++i],O_RDONLY); //if you enter < the file is in reading mode
            if(fd_in < 0){
                perror("Erreur open file");
                return;
             }
        }else if(strcmp(buf[i],">")==0){
            fd_out = open(buf[++i],O_WRONLY|O_TRUNC); //if you enter > the file in write mode
            if(fd_out<0){
                perror ("Erreur open file");
                return;
            }
        }else{
            new_buf[j++]=buf[i]; // stores orders without redirection

        }
    }
    pid = fork();

    if (pid < 0){
        perror("fork failed");
        exit(EXIT_FAILURE); 
    }if(pid !=0){ // father
        if(clock_gettime(CLOCK_REALTIME,&start)==-1){
            perror("clock");
            exit(EXIT_FAILURE);

        }wait(&status);
        if(clock_gettime(CLOCK_REALTIME,&stop)==-1){
            perror("clock");
            exit(EXIT_FAILURE);
        }
            time_execution_ns = stop.tv_nsec - start.tv_nsec;
            time_execution_s = stop.tv_sec - start.tv_sec;
            time_execution_ms = time_execution_s*1000 +  time_execution_ns*pow(10,-6); // convert to ms
        char msg [100];
        int len = 0;
        if(WIFEXITED(status)){
            len = snprintf(msg, sizeof(msg), "[exit: %d|time: %dms] ", WEXITSTATUS(status),time_execution_ms);
        }if (WIFSIGNALED(status)){
            len = snprintf(msg, sizeof(msg), "[sig: %d|time: %dms] ", WTERMSIG(status),time_execution_ms);

        }write(1, msg, len);
        
    }

    else{ // son
        //printf("%d\n",getpid()); // give us the process numbers
        //sleep(80); // we put the program to sleep while it takes time to kill the command
        if(fd_in != -1){
            dup2(fd_in,STDIN_FILENO); // the descriptor is duplicated on the input 
            close(fd_in);
        }
        if(fd_out != -1){
           dup2(fd_out,STDOUT_FILENO); // the descriptor is duplicated on the output
            close(fd_out); 
        }
        execvp(new_buf[0], new_buf);
        perror("execvp failed");
        exit(EXIT_FAILURE);  
    }
    free(new_buf); //allows you to free the memory given to the new buffer
}


char ** SeparateCommande (char* commande){
    char ** argv;
    argv = (char**)malloc(20*sizeof(char*)); // gives memory for 20 char pointers i.e. argv can store va token 
    int i = 0; 
    char * tab = strtok (commande," "); //will extract the command tokens using space as a separator. Will replace spaces with \0 and return a pointer to the token 
    while(tab !=0){
        argv[i] = (char*)malloc(20*sizeof(char));
        strcpy(argv[i],tab);
        tab = strtok(NULL," "); //allows you to recover the next token
        i++;
    }
    argv[i-1][strlen(argv[i-1])-1] = '\0'; // remove the \n character
    argv[i] = NULL;
    return  argv;

}
             