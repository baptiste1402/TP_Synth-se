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
    memset(buf,0,BUFFSIZE); // remplie buf de zéro pour éviter des erreurs 
       while(1){ // boucle infinie qui gère les commandes 
        write(1,"enseash %",strlen("enseash %"));
        commande_size = read(0,buf,BUFFSIZE); // permet de savoir le nombre de caractère obtenue 
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
    
    if(strcmp("exit",buf[0])==0 || commande_size ==0 || new_buf[0] == 0){
        write(1,"en revoir",strlen("en revoir"));
        exit(EXIT_FAILURE);}
    for(int i = 0; buf[i]!= NULL; i++){
        if(strcmp(buf[i],"<")==0){
            fd_in = open(buf[++i],O_RDONLY); // si on entre < le fichier est mode lecture 
            if(fd_in < 0){
                perror("Erreur open file");
                return;
             }
        }else if(strcmp(buf[i],">")==0){
            fd_out = open(buf[++i],O_WRONLY|O_TRUNC); // si on entre > le fichier en mode écriture
            if(fd_out<0){
                perror ("Erreur open file");
                return;
            }
        }else{
            new_buf[j++]=buf[i]; // stocke les commandes sans redirection 

        }
    }
    pid = fork();

    if (pid < 0){
        perror("fork failed");
        exit(EXIT_FAILURE); 
    }if(pid !=0){ // père
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
            time_execution_ms = time_execution_s*1000 +  time_execution_ns*pow(10,-6); // convertion en ms
        char msg [100];
        int len = 0;
        if(WIFEXITED(status)){
            len = snprintf(msg, sizeof(msg), "[exit: %d|time: %dms] ", WEXITSTATUS(status),time_execution_ms);
        }if (WIFSIGNALED(status)){
            len = snprintf(msg, sizeof(msg), "[sig: %d|time: %dms] ", WTERMSIG(status),time_execution_ms);

        }write(1, msg, len);
        
    }

    else{ // fils
        //printf("%d\n",getpid()); // nous donnes le numéros du processus 
        //sleep(80); // on endort le programme le temps de kill la commande
        if(fd_in != -1){
            dup2(fd_in,STDIN_FILENO); // le descripteur est dupliqué sur l'entrée 
            close(fd_in);
        }
        if(fd_out != -1){
           dup2(fd_out,STDOUT_FILENO); // le descripteur est dupliqué sur la sortie
            close(fd_out); 
        }
        execvp(new_buf[0], new_buf);
        perror("execvp failed");
        exit(EXIT_FAILURE);  
    }
    free(new_buf); // permet de libérer la mémoire donnée au nouveau buffer
}


char ** SeparateCommande (char* commande){
    char ** argv;
    argv = (char**)malloc(20*sizeof(char*)); // donne de la mémoire pour 20 pointeur char càd argv peut stocker va token 
    int i = 0; 
    char * tab = strtok (commande," "); // va extrarire les tokens de commande en se utilisant espace comme séparateur. Va remplacer les espaces par des \0 et renvoie un pointeur vers le token  
    while(tab !=0){
        argv[i] = (char*)malloc(20*sizeof(char));
        strcpy(argv[i],tab);
        tab = strtok(NULL," "); // permet de récupérer le prochain token 
        i++;
    }
    argv[i-1][strlen(argv[i-1])-1] = '\0'; // enlève le caractère \n 
    return  argv;

}
             