#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define GREATING "BONJOUR SERVEUR"

int main (int argc, char* argv[]) {
    int cfd;
    int success;
    int port;
    struct sockaddr_in srv_addr;

    char requete[] = "Créer processus";

    if (argc != 3) {
     printf("USAGE: ./client <serveur_ip_addr> <serveur_port>\n");
     exit(EXIT_FAILURE);
    }

    cfd = socket(AF_INET, SOCK_STREAM, 0);

    if (cfd < 0){
      printf("Le SE n'a pas pu créer la socket %d\n", cfd);
      exit(EXIT_FAILURE);
    }

    port = atoi(argv[2]);

    srv_addr.sin_family = AF_INET;
    srv_addr.sin_port = htons (port);
    inet_aton(argv[1], (struct in_addr *)&srv_addr.sin_addr.s_addr);

    success = connect(cfd, (struct sockaddr *) &srv_addr, sizeof(struct sockaddr_in));
    if (success < 0) {
        printf("Impossible de se connecter au serveur %s:%d error %d\n", argv[1], port, success);
        exit(EXIT_FAILURE);
    }
    
    int i = 1;

    while(1) {
        send(cfd, requete, sizeof(requete), 0);
        printf("%3d Requête envoyée: %s !\n", i, requete);
        i++;
        sleep(10);
    }

    close(cfd);
    return(0);
}
