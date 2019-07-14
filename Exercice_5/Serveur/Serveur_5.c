#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h> 
#include <sys/shm.h>  
#include <sys/sem.h>
#include <errno.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>

#define MY_ADDR "127.0.0.1"
#define MY_PORT 56789
#define LISTEN_BACKLOG 50

union senum {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};


int main(int argc, char* argv[])
{
    int sfd;
    int cfd;
    int i;

    struct sockaddr_in my_addr;
    struct sockaddr_in peer_addr;

    socklen_t peer_addr_size;
    pid_t child, pid1, pid2;

    char buffer[25];

    char requete[25];

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) {
        printf("Le SE n'a pas pu créer la socket %d\n", sfd);
        exit(EXIT_FAILURE);
    }

    memset(&my_addr, 0, sizeof(struct sockaddr_in));

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons (MY_PORT);
    inet_aton(MY_ADDR, (struct in_addr *)&my_addr.sin_addr.s_addr);

    if (bind(sfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_in)) < 0) {
        printf("bind error\n");
        exit(EXIT_FAILURE);
    }

    if (listen(sfd, LISTEN_BACKLOG) < -1) perror("listen\n");

    peer_addr_size = sizeof(struct sockaddr_in);

    while (1) {
        cfd = accept(sfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
        
        if (cfd < 0) {
            perror("accept\n");
            exit(EXIT_FAILURE);
        }

        child = fork();

        if (child < 0) {
            perror("Erreur de création du fils\n");
        }
        if (child == 0) {
            printf("Identité du client %d\n", peer_addr.sin_port);
            i = 1;
            while (1) {
                read(cfd, requete, sizeof(requete));
                printf("Requête reçue : %s\n", requete);
                int status;
                union senum sem_arg;

                key_t semKey = ftok("semaphore", 19);
                    
                int semId = semget(semKey, 1, 0666|IPC_CREAT);

                sem_arg.val = 1;
                if (semctl(semId, 0, SETVAL, sem_arg) == -1) {
                    perror("semctl");
                    exit(EXIT_FAILURE);
                }

                key_t shmKey = ftok("ShM", 12);

                int shmId = shmget(shmKey, sizeof(int), 0666|IPC_CREAT);

                int *a = (int*) shmat(shmId, (void*)0, 0);

                *a = 0;

                pid1 = fork();
                if (pid1 < 0) {
                    perror("Erreur de création du processus 1\n");
                    exit(EXIT_FAILURE);
                }
                    
                if(pid1 == 0) execv("./w", argv);
                else {
                    pid2 = fork();
                    if (pid2 < 0) {
                        perror("Erreur de création du processus 2\n");
                        pid1 = wait(&status);
                        exit(EXIT_FAILURE);
                    }
                    
                    if(pid2 == 0) execv("./w", argv);
                    else {
                        pid1 = wait(&status);
                        pid2 = wait(&status);
                        printf("Valeur Finale: %d\n", *a);

                        shmdt(a);
                        shmctl(shmId, IPC_RMID, NULL);
                            
                        if (semctl(semId, 0, IPC_RMID, NULL) == -1) {
                            perror("semctl");
                            exit(EXIT_FAILURE);
                        }
                    }
                }
                printf("Fin du traitement de la requête %d\n\n", i);
                i++;
            }

            printf("Fin !");

            printf("\n");
            close(sfd);
            break;
        }
        else {
            close(cfd);
        }
    }
}
