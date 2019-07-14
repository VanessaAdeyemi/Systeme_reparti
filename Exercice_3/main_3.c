#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/sem.h>
#include <errno.h>

#define SPACE ' '

//printcol: affiche la chaîne str dans un espace de colWidth caractères à partir de la droite 
int printcol(const int colWidth, const int nb);


int nbChiffre(int nb);

int main(int argc, char* argv[])
{
    FILE* fichierMatrice = NULL;
    int* matrice = NULL;
    int ordreMatrice, nbElementMatrice, tailleColonne = 0, status;
    pid_t** pid_tTab = NULL;
    int i, j, a, b;
    char* argp[] = {"calcul", NULL, NULL, NULL};
    
    char* chO, *chI, *chJ = NULL;
    
    int shmidMatrice;
    key_t keyMatrice;
  
    if (argc != 2 ) {
        printf("USAGE: <fichier_matrice>\n");
        exit(EXIT_FAILURE);
    }

//Lecture du fichier
    printf("\n");
    fichierMatrice = fopen(argv[1], "r");
    
    if (fichierMatrice == NULL) {
        printf("\tErreur: Le fichier %s n'exite pas ou ne peut être lu !\n", argv[1]);
        exit(EXIT_FAILURE);
    }

//Lecture de l'ordre de la matrice    
    fscanf(fichierMatrice, "%d ", &ordreMatrice);
    
    if (ordreMatrice < 2) {
        printf("\tErreur: L'ordre de la matrice doit être supérieur à 2 !\n");
        fclose(fichierMatrice);
        exit(EXIT_FAILURE);
    }
    
    nbElementMatrice = ordreMatrice * ordreMatrice;

//Création de la matrice de calcul en mémoire partagée
    keyMatrice = ftok("shmMatrice", 19);
    shmidMatrice = shmget(keyMatrice, 2 * nbElementMatrice * sizeof(int), 0666|IPC_CREAT);
    matrice = (int*) shmat(shmidMatrice,(void*)0,0); 
    
    if (matrice == NULL) {
        printf("Allocation de mémoire impossible !\n");
        fclose(fichierMatrice);
        exit(EXIT_FAILURE);
    }

//Lecture de la matrice dans le fichier et remplissage de la matrice de calcul    
    for (i = 0; i < ordreMatrice; i++) {
        for (j = 0; j< ordreMatrice; j++) {
            fscanf(fichierMatrice, "%d ", &matrice[i*ordreMatrice + j]);
            
            if (tailleColonne < nbChiffre(matrice[i*ordreMatrice + j]))
                tailleColonne = nbChiffre(matrice[i*ordreMatrice + j]);
            
//Controle qu'on récupere une matrice carrée d'ordre ordreMatrice dans le fichier
            if (feof(fichierMatrice) && j != ordreMatrice-1) {
                fclose(fichierMatrice);
                
                shmdt(matrice); 
                shmctl(shmidMatrice, IPC_RMID, NULL);
                
                printf("Erreur: Matrice carrée en entrée incomplète !\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    
    fclose(fichierMatrice);
    printf("Fin lecture de la matrice en fichier !\n\nOrdre de la matrice: %d\n\n", ordreMatrice);
    
//Affichage de la matrice de calcul
    for (i = 0; i < ordreMatrice; i++) {
    
        putchar('+');
        
        for (j = 0; j < ordreMatrice; j++) {
            for (a = 0; a < tailleColonne; a++)
                putchar('-');
            putchar('+');
        }
        printf("\n|");
        
        for (j = 0; j < ordreMatrice; j++) {
            printcol(tailleColonne, matrice[i*ordreMatrice + j]);
            putchar('|');
        }
        printf("\n");       
    }
    putchar('+');
    for (i = 0; i < ordreMatrice; i++) {
        for (j = 0; j < tailleColonne; j++)
            putchar('-');
        putchar('+');
    }
    printf("\n\n");
    
//Création des pid_t des processus
    pid_tTab = malloc(ordreMatrice * sizeof(pid_t*));
    
    if (pid_tTab == NULL) {
        printf("Allocation de mémoire impossible !\n");
        
        shmdt(matrice); 
        shmctl(shmidMatrice, IPC_RMID, NULL);
        
        exit(EXIT_FAILURE);
    }
    
    for (i = 0; i < ordreMatrice; i++) {
        pid_tTab[i] = malloc(ordreMatrice * sizeof(pid_t));
        
        if (pid_tTab[i] == NULL) {
            printf("Allocation de mémoire impossible !\n");
            
            for (j = 0; j < i; j++)
                free(pid_tTab[j]);
            
            free(pid_tTab);
            shmdt(matrice); 
            shmctl(shmidMatrice, IPC_RMID, NULL);
            
            exit(EXIT_FAILURE);
        }
    }
 
//Création des chaines pour la transmission des paramètres aux processus    

    chO = malloc((1 + nbChiffre(ordreMatrice)) * sizeof(char));
    
    if (chO == NULL) {
        printf("Allocation de mémoire impossible !\n");
            for (i = 0; i < ordreMatrice; i++)
                free(pid_tTab[i]);
                
        free(pid_tTab);
        
        shmdt(matrice); 
        shmctl(shmidMatrice, IPC_RMID, NULL);
        
        exit(EXIT_FAILURE);
    }
    
    sprintf(chO, "%d", ordreMatrice);
    argv[1] = chO;
    
    chI = malloc((1 + nbChiffre(ordreMatrice)) * sizeof(char));
    
    if (chI == NULL) {
        printf("Allocation de mémoire impossible !\n");
            for (i = 0; i < ordreMatrice; i++)
                free(pid_tTab[i]);
                
        free(chO);
            
        free(pid_tTab);
            
        shmdt(matrice); 
        shmctl(shmidMatrice, IPC_RMID, NULL);
        
        exit(EXIT_FAILURE);
    }
    
    chJ = malloc((1 + nbChiffre(ordreMatrice)) * sizeof(char));
    
    if (chJ == NULL) {
        printf("Allocation de mémoire impossible !\n");
            for (i = 0; i < ordreMatrice; i++)
                free(pid_tTab[i]);
  
        free(chO);
        free(chI);
            
        free(pid_tTab);
            
        shmdt(matrice); 
        shmctl(shmidMatrice, IPC_RMID, NULL);
            
        exit(EXIT_FAILURE);
    }
    
//Création des processus et passage de leurs paramètres
    for (i = 0; i < ordreMatrice; i++) {
        for (j = 0; j < ordreMatrice; j++) {

            sprintf(chI, "%d", i);
            argv[2] = chI;
            sprintf(chJ, "%d", j);
            argv[3] = chJ;
                        
            pid_tTab[i][j] = fork();
            
            if (pid_tTab[i][j] < 0) {
                printf("Création du processus pour le calcul du c[%d,%d] impossible \n", i, j);
                    
                for (a = 0; a <= i; a++) {
                    for (b = 0; b < j; b++)
                        wait(&status);
                }
                    
                for (a = 0; a < ordreMatrice; a++)
                    free(pid_tTab[i]);
                        
                free(chO);
                free(chI);
                free(chJ);
                free(pid_tTab);
                
                shmdt(matrice); 
                
                shmctl(shmidMatrice, IPC_RMID, NULL); 
                
                exit(EXIT_FAILURE);
            }
            
            if (pid_tTab[i][j] == 0) {
                execv("./calcul", argv);
            }
            sleep(2);
        }
    }
    
    
//Arret des processus et du programme
    if (getpid() > 0) {
        
        tailleColonne = 0;
        
        for (i = 0; i < nbElementMatrice; i++) {
            if (tailleColonne < nbChiffre(matrice[i+nbElementMatrice]))
                tailleColonne = nbChiffre(matrice[i+nbElementMatrice]);
        }
        
//Affichage de la matrice de calcul
    for (i = 0; i < ordreMatrice; i++) {
    
        putchar('+');
        
        for (j = 0; j < ordreMatrice; j++) {
            for (a = 0; a < tailleColonne; a++)
                putchar('-');
            putchar('+');
        }
        printf("\n|");
        
        for (j = 0; j < ordreMatrice; j++) {
            printcol(tailleColonne, matrice[i*ordreMatrice + j + nbElementMatrice]);
            putchar('|');
        }
        printf("\n");       
    }
    putchar('+');
    for (i = 0; i < ordreMatrice; i++) {
        for (j = 0; j < tailleColonne; j++)
            putchar('-');
        putchar('+');
    }
    printf("\n\n");

//FArrêt du programme
       
        free(chO);
        free(chI);
        free(chJ);
        
        for (i = 0; i < ordreMatrice; i++) {
            for(j = 0; j < ordreMatrice; j++)
                wait(&status);
                
            for(j = 0; j < ordreMatrice; j++) printf("Arrêt du processus %d %d\n", status, pid_tTab[i][j]);
                    
            free(pid_tTab[i]);        
        }
            
        free(pid_tTab);
        shmdt(matrice); 
        shmctl(shmidMatrice, IPC_RMID, NULL);
        putchar('\n');
    }
}

int nbChiffre(int nb)
{
    int i;
    for (i = (nb <= 0) ? 1 : 0; nb != 0; i++) nb /= 10;
    return i;   
}

int printcol(const int colWidth, const int nb)
{
    const int strWidth = nbChiffre(nb);
    int nbrSpace;

    if (colWidth > 0 && strWidth <= colWidth){
        for (nbrSpace = colWidth - strWidth; nbrSpace > 0; nbrSpace--)
            putchar(SPACE);
        printf("%d", nb);
        return 1;
    }
    else return 0;
}
