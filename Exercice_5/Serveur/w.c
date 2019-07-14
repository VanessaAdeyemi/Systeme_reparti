#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <stdio.h> 
#include <sys/sem.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>

#define ITER 3

union senum {
    int val;
    struct semid_ds  *buf;
    unsigned short   *array;
    struct seminfo   *__buf;
};


int main()
{
    key_t shmKey = ftok("ShM", 12);
    int i;

    struct sembuf sb = {0, 0, 0};
    union senum sem_arg;

    key_t semKey = ftok("semaphore", 19);

    int semId = semget(semKey, 1, 0666);

    int shmId = shmget(shmKey, sizeof(int), 0666);

    int* a = (int*) shmat(shmId, (void*)0, 0);

    for (i = 0; i < ITER; i++) {
        sb.sem_op = -1;
        if (semop(semId, &sb, 1) == -1) {
            perror("sempo");
            exit(EXIT_FAILURE);
        }

        printf("Proc w %d %d\n", getpid(), *a);
        (*a)++;
        sleep(2);
        
        sb.sem_op = 1;
        if (semop(semId, &sb, 1) == -1) {
            perror("sempo");
            exit(EXIT_FAILURE);
        }
    }
    shmdt(a);
    return 0;
}
