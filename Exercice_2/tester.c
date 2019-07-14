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

#define ITER 5


union semun 
{
	int val;
	struct semid_ds *buf;
	unsigned short *array;
	struct seminfo *__buf;
};

int main()
{
	key_t key = ftok("shm_file",70);
	int i ;
	int a= 0;
	struct sembuf sb = {0, 0, 0};
	union semun sem_arg;

	key_t sem_key = ftok("semfile",80);
	int semid = semget(sem_key,1,0666);

	int shmid = shmget(key,1024,0666); 

	char *str = (char*) shmat(shmid,(void*)0,0);

	for(i=0; i<ITER; i++)
	{
		sb.sem_op = -1;
		if(semop(semid, &sb,1) == -1)
		{
			perror("sempo");
			exit(1);
		}
		
		a= atoi(str);
		printf("proc tester %d %d\n", getpid(), a);
		a= a+1;
		sleep(a);

		sprintf(str, "%d", a);
		sb.sem_op = 1;
		if(semop(semid, &sb,1) == -1 )
		{
			perror("sempo");
			exit(1);
		}
	}
	
	shmdt(str);
	return 0;
}
 
