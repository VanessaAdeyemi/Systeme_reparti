#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    pid_t pid1, pid2, pid3;
    int status;
    char *argp1[] = {"Q1", argv[1], NULL};Q3
    if(argc!=4){
       printf("USAGE projet1 data1.in data2.in data3.in\n");
       exit(1);
    }

    pid1 = fork();

    if(pid1 < 0)
    {
        perror("erreur de la creation du processus\n");
        exit (EXIT_FAILURE);
    }
    
    if(pid1 == 0)
    {
        execv("./Q1", argp1);
    }

    else
    {
         pid2 = fork();
         if(pid2 < 0)
         {
             perror("erreur de la creation du second processus\n");
             pid1 = wait(&status);
             exit (EXIT_FAILURE);
         }

         if(pid2 == 0)
         {
             execv("./Q1", argp2);
         }
    
         else
         {
             pid3 = fork();
             if(pid3 < 0)
             {
                 perror("erreur de la creation du troisieme processus");
                 pid1 = wait(&status);
                 pid2 = wait(&status);
                 exit (EXIT_FAILURE);
             }

             if(pid3 == 0)
             {
                 execv("./Q1", argp3);
             }
 
             else
             {
                 pid1 = wait(&status);
                 printf("Status de l'arret du fils %d %d\n", status, pid1);
                 pid2 = wait(&status);
		 printf("Status de l'arret du fils %d %d\n", status, pid2);
                 pid3 = wait(&status);
		 printf("Status de l'arret du fils %d %d\n", status, pid3);
             }
         }
    }
}
