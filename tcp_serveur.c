#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include "client.h"
#include <time.h>
#include <pthread.h>
#define MAXPENDING 10    /* Max connection requests */
#define BUFFSIZE 1000

int serversock, clientsock;
int nb_clients = 0;
pthread_t tid;
pthread_t serverthreads[100];
void Die(char *mess) { perror(mess); exit(EXIT_FAILURE); }

Client liste_clients[100];
struct sockaddr_in echoserver, echoclient;
time_t rawtime;
char * temps;
void*HandleClient(void* param) {
    int sock = *((int*)param);
    char buffer[50];
    int received = -1;
    /* Receive message */
    if ((received = recv(sock, buffer, 50, 0)) < 0) {
        Die("Failed to receive initial bytes from client");
    }
    char*requete = NULL;
    char*client = NULL;
    char*password = NULL;
    char* pend = NULL;
    int compte = -1;

    while (received > 0) {
        time(&rawtime);
        temps = ctime(&rawtime);
        buffer[received] = '\0';
        requete = strtok(buffer, " \t");
        if (!strcmp("exit", requete)) {
            if (send(sock, "GOODBYE", 8, 0) != 8) {
                Die("Failed to send bytes to client\n");
            }
            fprintf(stdout, "Client disconnected: %s\n",inet_ntoa(echoclient.sin_addr));
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fprintf(stderr,"Failed to receive additional bytes from client\n");
                break;
            }
            break;
        }

        //r�cup�ration id_client
        client = strtok(NULL, " \t");
        if (!client) {
            if (send(sock, "KO", received, 0) != received) {
                Die("Failed to send bytes to client");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fprintf(stderr,"Failed to receive additional bytes from client\n");
                break;
            }
            fprintf(stderr,"Identifiant invalide.\n");
            continue;
        }

        //r�cup�ration num�ro de compte
        char* temp = strtok(NULL, " \t");
        if (temp) compte = strtol(temp, NULL, 10);
        else {
            if (send(sock, "KO", 3, 0) != 3) {
                Die("Failed to send bytes to client\n");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fprintf(stderr,"Failed to receive additional bytes from client\n");
                break;
            }
            fprintf(stderr,"Numéro de compte invalide.\n");
            continue;
        }
        
        if (compte<0) {
            fprintf(stderr, "compte\n");
            if (send(sock, "KO", 3, 0) != 3) {
                Die("Failed to send bytes to client\n");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fprintf(stderr,"Failed to receive additional bytes from client\n");
                break;
            }
            fprintf(stderr,"Num�ro de compte invalide.");
            continue;
        }

        //r�cup�ration mot de passe
        password = strtok(NULL, " \t");
        if (!password) {
            fprintf(stderr, "compte\n");
            if (send(sock, "KO", 3, 0) != 3) {
                Die("Failed to send bytes to client\n");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fprintf(stderr,"Failed to receive additional bytes from client\n");
                break;
            }
            fprintf(stderr,"Mot de passe invalide.\n");
            continue;
        }

        int i = 0;
        //on v�rifie que le client est bien dans la base de donn�e
        while (i < nb_clients && strcmp(liste_clients[i].id_client, client)) i++;
        if (i >= nb_clients) {
            if (send(sock, "KO", 3, 0) != 3) {
                fprintf(stderr,"Failed to receive additional bytes from client\n");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                Die("Failed to receive additional bytes from client\n");
            }
            continue;
        }
        //on v�rifie que le mdp correspond bien
        if (strcmp(liste_clients[i].password, password)) {
            if (send(sock, "KO", 3, 0) != 3) {
                Die("Failed to send bytes to client\n");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fprintf(stderr,"Failed to receive additional bytes from client\n");
                break;
            }
            continue;
        }


        //on verifie quelle operation
        if (!strcmp("AJOUT", requete)) {

            //on r�cup�re la somme � ajouter et on v�rifie que le num�ro de compte est valide
            temp = strtok(NULL, " ");
            int somme = 0;
            if (temp) somme = strtol(temp, &pend, 10);
            else {
                if (send(sock, "KO", 3, 0) != 3) {
                    Die("Failed to send bytes to client\n");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fprintf(stderr,"Failed to receive additional bytes from client\n");
                    break;
                }
                continue;
            }
            if (compte<liste_clients[i].nb_compte) liste_clients[i].compte[compte].montant += somme;
            else{
                if (send(sock, "KO", 3, 0) != 3) {
                    Die("Failed to send bytes to client\n");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fprintf(stderr,"Failed to receive additional bytes from client\n");
                    break;
                }
                fprintf(stderr,"Num�ro de compte invalide.\n");
                continue;
            }

            //on met � jour le fichier liste_operations_id.txt
            char nom_fichier[100];
            sprintf(nom_fichier, "OperationsClients/liste_operations_%s.txt", liste_clients[i].id_client);
            FILE* liste_op = fopen(nom_fichier, "a");
            if (liste_op == NULL) {
                fprintf(stderr, "Impossible d'ouvrir le fichier liste_operations.txt");
                if (send(sock, "KO", 3, 0) != 3) {
                    Die("Failed to send bytes to client\n");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fprintf(stderr,"Failed to receive additional bytes from client\n");
                    break;
                }
                break;
            }
            fprintf(liste_op, "%d;AJOUT;%d;%s", compte, somme,temps);
            fclose(liste_op);

            //on met � jour le fichier bdd_clients.csv
            FILE* fichier = fopen("bdd_clients.csv", "w");
            if (fichier == NULL) {
                fprintf(stderr, "Impossible d'ouvrir le fichier bdd_clients.csv\n");
                if (send(sock, "KO", 3, 0) != 3) {
                    Die("Failed to send bytes to client");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fprintf(stderr,"Failed to receive additional bytes from client\n");
                    break;
                }
                break;
            }

            fprintf(fichier, "id_client;password;nb_comptes;solde\n");
            for (int j = 0; j < nb_clients; j++) {
                fprintf(fichier, "%s;%s;%d;", liste_clients[j].id_client, liste_clients[j].password, liste_clients[j].nb_compte);
                for (int k = 0; k < liste_clients[j].nb_compte; k++) {
                    fprintf(fichier, "%d;", liste_clients[j].compte[k].montant);
                }
                fprintf(fichier, "\n");
            }
            fclose(fichier);
        
            //on envoie un message de confirmation
            if (send(sock, "OK", 3, 0) != 3) {
                Die("Failed to send bytes to client\n");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fprintf(stderr,"Failed to receive additional bytes from client\n");
                break;
            }
        }

        else if (!strcmp("RETRAIT", requete)) {

            //on r�cup�re la somme � ajouter et on v�rifie que le num�ro de compte est valide
            temp = strtok(NULL, " ");
            int somme = 0;
            if (temp) somme = strtol(temp, &pend, 10);
            else {
                if (send(sock, "KO", 3, 0) != 3) {
                    Die("Failed to send bytes to client\n");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fprintf(stderr,"Failed to receive additional bytes from client\n");
                    break;
                }
                continue;
            }
            if (compte<liste_clients[i].nb_compte) liste_clients[i].compte[compte].montant -= somme;
            else{
                if (send(sock, "KO", 3, 0) != 3) {
                    Die("Failed to send bytes to client\n");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fprintf(stderr,"Failed to receive additional bytes from client\n");
                    break;
                }
                fprintf(stderr,"Num�ro de compte invalide.");
                continue;
            }

                        //on met � jour le fichier liste_operations_id.txt
            char nom_fichier[100];
            sprintf(nom_fichier, "OperationsClients/liste_operations_%s.txt", liste_clients[i].id_client);
            FILE* liste_op = fopen(nom_fichier, "a");
            if (liste_op == NULL) {
                fprintf(stderr, "Impossible d'ouvrir le fichier liste_operations.txt");
                if (send(sock, "OK", 3, 0) != 3) {
                Die("Failed to send bytes to client");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fprintf(stderr,"Failed to receive additional bytes from client\n");
                    break;
                }
                break;
            }
            fprintf(liste_op, "%d;RETRAIT;%d;%s", compte, somme,temps);
            fclose(liste_op);

            //on met � jour le fichier bdd_clients.csv
            FILE* fichier = fopen("bdd_clients.csv", "w");
            if (fichier == NULL) {
                fprintf(stderr, "Impossible d'ouvrir le fichier bdd_clients.csv");
                if (send(sock, "OK", 3, 0) != 3) {
                    Die("Failed to send bytes to client");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fprintf(stderr,"Failed to receive additional bytes from client\n");
                    break;
                }
                break;
            }
            fprintf(fichier, "id_client;password;nb_comptes;solde\n");
            for (int j = 0; j < nb_clients; j++) {
                fprintf(fichier, "%s;%s;%d;", liste_clients[j].id_client, liste_clients[j].password, liste_clients[j].nb_compte);
                for (int k = 0; k < liste_clients[j].nb_compte; k++) {
                    fprintf(fichier, "%d;", liste_clients[j].compte[k].montant);
                }
                fprintf(fichier, "\n");
            }
            fclose(fichier);
        
            //on envoie un message de confirmation
            if (send(sock, "OK", 3, 0) != 3) {
                Die("Failed to send bytes to client\n");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fprintf(stderr,"Failed to receive additional bytes from client\n");
                break;
            }
        }


        else if (!strcmp("SOLDE", requete)) {
            char solde[200];
            if (compte>=liste_clients[i].nb_compte){
                if (send(sock, "KO", 3, 0) != 3) {
                    Die("Failed to send bytes to client\n");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fprintf(stderr,"Failed to receive additional bytes from client\n");
                    break;
                }
                fprintf(stderr,"Num�ro de compte invalide.\n");
                break;
            }

            //on lit dans le fichier la dernière opération
            char* line = NULL;
            size_t len = 0;
            char* date;
            char nom_fichier[100];
            sprintf(nom_fichier, "OperationsClients/liste_operations_%s.txt", client);
            FILE* liste_op = fopen(nom_fichier, "r");
            if (liste_op == NULL) {
                fprintf(stderr, "Impossible d'ouvrir le fichier liste_operations.txt");
                if (send(sock, "KO", 3, 0) != 3) {
                    Die("Failed to send bytes to client\n");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fprintf(stderr,"Failed to receive additional bytes from client\n");
                    break;
                }
                break;
            }
            while ((getline(&line, &len, liste_op)) != -1) {
                if (strtol(line, NULL, 10) == compte){
                    strtok(line, ";");
                    strtok(NULL, ";");
                    strtok(NULL, ";");
                    date = strtok(NULL, ";");
                    date[strlen(date) - 1] = '\0';
                }
            }
            fclose(liste_op);
            sprintf(solde, "RES_SOLDE %d %s",liste_clients[i].compte[compte].montant, date);
            if (send(sock, solde, 200, 0) != 200) {
                Die("Failed to send bytes to client");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fprintf(stderr,"Failed to receive additional bytes from client\n");
                break;
            }
        }

        else if (!strcmp("OPERATIONS", requete)) {
            char* line = NULL;
            char message[BUFFSIZE];
            int count = 0;
            size_t len = 0;
            char* operation[10];
            sprintf(message, "RES_OPERATIONS\n");
            char nom_fichier[100];
            sprintf(nom_fichier, "OperationsClients/liste_operations_%s.txt", liste_clients[i].id_client);
            FILE* liste_op = fopen(nom_fichier, "r");
            if (liste_op == NULL) {
                fprintf(stderr, "Impossible d'ouvrir le fichier liste_operations.txt");
                if (send(sock, "KO", 3, 0) != 3) {
                    Die("Failed to send bytes to client");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fprintf(stderr,"Failed to receive additional bytes from client\n");   
                    break;
                }
                break;
            }
            while ((getline(&line, &len, liste_op)) != -1) {
                if (strtol(line, NULL, 10) == compte){
                    if (count == 10){
                        for (int j = 0; j < 9; j++) {
                            operation[j] = malloc(sizeof(char) * strlen(operation[j + 1]));
                            strcpy(operation[j], operation[j + 1]);
                        }
                        count = 9;
                    }
                    operation[count] = malloc(sizeof(char) * strlen(line));
                    strcpy(operation[count], line);
                    count++;
                }
            }
            fclose(liste_op);
            for (int j = 0; j < count; j++) {
                strcat(message, operation[j]);
            }
            if (send(sock, message, BUFFSIZE, 0) != BUFFSIZE) {
                Die("Failed to send bytes to client");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fprintf(stderr,"Failed to receive additional bytes from client\n");   
                break;
            }
        }
        else{
            if (send(sock, "KO", 3, 0) != 3) {
                Die("Failed to send bytes to client");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fprintf(stderr,"Failed to receive additional bytes from client\n");   
                break;
            }
            fprintf(stderr,"Requ�te invalide.\n");
        }
    }
    close(sock);
    return NULL;
}

void INThandler(int sig) {
    signal(sig, SIG_IGN);
    if (serversock) close(serversock);
    if (clientsock) close(clientsock);
    printf("\nGoodbye\n");
    exit(0);
}

int main(int argc, char *argv[]) {
//initialisation clients de la banque par lecture dans un fichier
    signal(SIGINT, INThandler);
    time(&rawtime);
    temps = ctime(&rawtime);
    FILE* bdd_clients = fopen("bdd_clients.csv", "r+");
    char* line = NULL;
    size_t len = 0;
    int i = 0;
    if (bdd_clients == NULL) {
        bdd_clients = fopen("bdd_clients.csv", "w");
        fprintf(bdd_clients, "id_client,password,nb_comptes,solde");
    }
    else {
        getline(&line, &len, bdd_clients);
        while (getline(&line, &len, bdd_clients) != -1){
            if (!strcmp(line,"\n")) break;
            char* client = strtok(line, "  , ; \n");
            if (!client) continue;
            char* password = strtok(NULL, "  , ; \n");
            if (!password) continue;
            liste_clients[i].id_client = (char*) malloc(strlen(client) + 1);
            liste_clients[i].password = (char*) malloc(strlen(password) + 1);
            if (client) strcpy(liste_clients[i].id_client,client);
            if (password) strcpy(liste_clients[i].password,password);
            int nb_compte = strtol(strtok(NULL, "  , ; \n"), NULL, 10);
            if (!nb_compte) continue;
            liste_clients[i].compte = (Compte*) malloc(nb_compte);
            liste_clients[i].nb_compte = nb_compte;
            for (int j = 0; j < nb_compte; j++) {
                char * p = strtok(NULL, "  { , } ; \n");
                if (p) liste_clients[i].compte[j].montant = strtol(p, NULL, 10);
                else liste_clients[i].compte[j].montant = 0;
            }
            char nom_fichier[100];
            sprintf(nom_fichier, "OperationsClients/liste_operations_%s.txt", client);
            if (access(nom_fichier, F_OK)) { //on verifie si le fichier existe
                FILE* op_clients = fopen(nom_fichier, "w");
                fclose(op_clients);
            }
            i++;
            nb_clients++;
        }
        for (int i = 0;i<nb_clients;i++){
            printf("id : %s  mdp : %s\t",liste_clients[i].id_client,liste_clients[i].password);
            for (int j = 0;j<liste_clients[i].nb_compte;j++) printf("%d ",liste_clients[i].compte[j].montant);
            printf("\n");
        }
    }
    fclose(bdd_clients);

    if (argc != 2) {
        fprintf(stderr, "USAGE: echoserver <port>\n");
        exit(EXIT_FAILURE);
    }
    /* Create the TCP socket */
    if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        Die("Failed to create socket");
    }
    /* Construct the server sockaddr_in structure */
    memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
    echoserver.sin_family = AF_INET;                  /* Internet/IP */
    echoserver.sin_addr.s_addr = htonl(INADDR_ANY);   /* Incoming addr */
    echoserver.sin_port = htons(atoi(argv[1]));       /* server port */
/* Bind the server socket */
    if (bind(serversock, (struct sockaddr *) &echoserver, sizeof(echoserver)) < 0) {
        Die("Failed to bind the server socket");
    }
    /* Listen on the server socket */
    if (listen(serversock, MAXPENDING) < 0) {
        Die("Failed to listen on server socket");
    }
    pthread_t tid[100];
    i = 0;
/* Run until cancelled */
    while (1) {
        // https://www.geeksforgeeks.org/handling-multiple-clients-on-server-with-multithreading-using-socket-programming-in-c-cpp/

        unsigned int clientlen = sizeof(echoclient);
        /* Wait for client connection */
        if ((clientsock = accept(serversock, (struct sockaddr *) &echoclient, &clientlen)) < 0) {
            Die("Failed to accept client connection");
        }
        fprintf(stdout, "Client connected: %s\n",
                        inet_ntoa(echoclient.sin_addr));
        if (pthread_create(&tid[i], NULL, &HandleClient, (void *) &clientsock) != 0) {
            Die("Failed to create thread\n");
        }
        i++;

        if (i>=MAXPENDING){
            i = 0;
            while (i < MAXPENDING) {
                pthread_join(tid[i++], NULL);
            }
            i = 0;
        }
    }
}
