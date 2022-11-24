#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include "client.h"
#include <time.h>
#define MAXPENDING 5    /* Max connection requests */
#define BUFFSIZE 1000

time_t rawtime;
struct tm* timeinfo;


void Die(char *mess) { perror(mess); exit(EXIT_FAILURE); }

Client liste_clients[100];
int nb_clients = 0;
int op[5] = {0,0,0,0,0};
void HandleClient(int sock) {
    char buffer[BUFFSIZE];
    int received = -1;
    /* Receive message */
    if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
        Die("Failed to receive initial bytes from client");
    }
    char*requete = NULL;
    char*client = NULL;
    char*password = NULL;
    char* pend = NULL;
    int compte = -1;
    /* Send bytes and check for more incoming data in loop */
    while (received > 0) {
        FILE* bdd_clients = fopen("bdd_clients.csv", "a");
        //r�cup�ration requ�te
        requete = strtok(buffer, " ");
        //r�cup�ration id_client
        client = strtok(NULL, " ");
        if (!client) {
            fclose(bdd_clients);
            fprintf(stderr, "id\n");
            if (send(sock, "KO", received, 0) != received) {
                fclose(bdd_clients);
                Die("Failed to send bytes to client");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fclose(bdd_clients);
                Die("Failed to receive additional bytes from client");
            }
            continue;
        }

        //r�cup�ration num�ro de compte
        char* temp = strtok(NULL, " ");
        if (temp) compte = strtol(temp, NULL, 10);
        else {
            if (send(sock, "KO", received, 0) != received) {
                fclose(bdd_clients);
                Die("Failed to send bytes to client");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fclose(bdd_clients);
                Die("Failed to receive additional bytes from client");
            }
            continue;
        }
        if (compte<0 || compte >5) {
            if (send(sock, "KO", received, 0) != received) {
                fclose(bdd_clients);
                Die("Failed to send bytes to client");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fclose(bdd_clients);
                Die("Failed to receive additional bytes from client");
            }
            continue;
        }

        //r�cup�ration mot de passe
        password = strtok(NULL, " ");
        if (!password) {
            if (send(sock, "KO", received, 0) != received) {
                fclose(bdd_clients);
                Die("Failed to send bytes to client");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fclose(bdd_clients);
                Die("Failed to receive additional bytes from client");
            }
            continue;
        }


        if (!strcmp("NEW", requete)) {
            fprintf(bdd_clients, "\n%s,%d,%s,{", client, compte, password);
            fprintf(stderr, "\n%s,%d,%s,{", client, compte, password);
            for (int j = 0; j < compte - 1; j++) fprintf(bdd_clients, "0,");
            fprintf(bdd_clients, "0}");
            if (send(sock, "OK", received, 0) != received) {
                fclose(bdd_clients);
                Die("Failed to send bytes to client");
            }
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fclose(bdd_clients);
                Die("Failed to receive additional bytes from client");
            }
            continue;
        }

        else {
            int i = 0;
            //on v�rifie que le client est bien dans la base de donn�e
            fprintf(stderr,"Valeur de i : %d\n",i);

            while (i < nb_clients && (strcmp(liste_clients[i].id_client, client))) {            fprintf(stderr,"Nom client : %d Dans la liste : %s Dans client : %s\n",(strcmp(liste_clients[i].id_client, client)),liste_clients[0].id_client,client);i++;}
            if (i >= nb_clients) {
                fprintf(stderr,"ID NOT OK\n");
                if (send(sock, "KO", received, 0) != received) {
                    fclose(bdd_clients);
                    Die("Failed to send bytes to client");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fclose(bdd_clients);
                    Die("Failed to receive additional bytes from client");
                }
                continue;
            }

            //on v�rifie que le mdp correspond bien
            if (strcmp(liste_clients[i].password, password)) {
                fprintf(stderr,"PASSWORD NOT OK\n");
                if (send(sock, "KO", received, 0) != received) {
                    fclose(bdd_clients);
                    Die("Failed to send bytes to client");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fclose(bdd_clients);
                    Die("Failed to receive additional bytes from client");
                }
            }


            //on verifie quelle operation
            if (!strcmp("AJOUT", requete)) {

                //on r�cup�re la somme � ajouter et on v�rifie que le num�ro de compte est valide
                temp = strtok(NULL, " ");
                int somme = 0;
                if (temp) somme = strtol(temp, &pend, 10);
                else {
                    fprintf(stderr,"AJOUT NOT OK\n");
                    if (send(sock, "KO", received, 0) != received) {
                        fclose(bdd_clients);
                        Die("Failed to send bytes to client");
                    }
                    if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                        fclose(bdd_clients);
                        Die("Failed to receive additional bytes from client");
                    }
                }
                liste_clients[i].compte[compte].montant += somme;

                //on met � jour la liste des op�rations
                time(&rawtime);
                timeinfo = localtime(&rawtime);
                if (op[compte] == 10) { //si on atteint la fin du tableau, on supprime le premier �lement et on r�arrange pour �tre tri�
                    op[compte] = 9;
                    for (int j = 0; j < 10 - 1; j++) liste_clients[i].compte[compte].liste[j] = liste_clients[i].compte[compte].liste[j + 1];
                }

                sprintf(liste_clients[i].compte[compte].liste[op[compte]].type, "Ajout");
                liste_clients[i].compte[compte].liste[op[compte]].montant = somme;
                sprintf(liste_clients[i].compte[compte].liste[op[compte]].time, "%s", asctime(timeinfo));
                liste_clients[i].compte[compte].liste[op[compte]].time[strlen(liste_clients[i].compte[compte].liste[op[compte]].time) - 1] = '\0'; //pour enlever le newline
                op[compte]++;

                if (send(sock, "OK", received, 0) != received) {
                    fclose(bdd_clients);
                    Die("Failed to send bytes to client");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fclose(bdd_clients);
                    Die("Failed to receive additional bytes from client");
                }
            }

            else if (!strcmp("RETRAIT", requete)) {

                //on r�cup�re la somme � retirer et on v�rifie que le num�ro de compte est valide
                temp = strtok(NULL, " ");
                int somme = 0;
                if (temp) somme = strtol(temp, &pend, 10);
                else {
                    if (send(sock, "KO", received, 0) != received) {
                        fclose(bdd_clients);
                        Die("Failed to send bytes to client");
                    }
                    if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                        fclose(bdd_clients);
                        Die("Failed to receive additional bytes from client");
                    }
                }
                liste_clients[i].compte[compte].montant -= somme;

                //on met � jour la liste des op�rations

                if (op[compte] == 10) { //si on atteint la fin du tableau, on supprime le premier �lement et on r�arrange pour �tre tri�
                    op[compte] = 9;
                    for (int j = 0; j < 10 - 1; j++) liste_clients[i].compte[compte].liste[j] = liste_clients[i].compte[compte].liste[j + 1];
                }
                time(&rawtime);
                timeinfo = localtime(&rawtime);
                sprintf(liste_clients[i].compte[compte].liste[op[compte]].type, "Retrait");
                liste_clients[i].compte[compte].liste[op[compte]].montant = somme;
                sprintf(liste_clients[i].compte[compte].liste[op[compte]].time, "%s", asctime(timeinfo));
                liste_clients[i].compte[compte].liste[op[compte]].time[strlen(liste_clients[i].compte[compte].liste[op[compte]].time) - 1] = '\0'; //pour enlever le newline
                op[compte]++;

                if (send(sock, "OK", received, 0) != received) {
                    fclose(bdd_clients);
                    Die("Failed to send bytes to client");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fclose(bdd_clients);
                    Die("Failed to receive additional bytes from client");
                }
                //on trouve le client dans la base et on verifie que c'est le bon password
            }

            else if (!strcmp("SOLDE", requete)) {
                char solde[200];
                if (strcmp(liste_clients[i].compte[compte].liste[op[compte] - 1].time, "")) sprintf(solde, "RES_SOLDE %d %s\n", liste_clients[i].compte[compte].montant, liste_clients[i].compte[compte].liste[op[compte] - 1].time);
                else sprintf(solde, "RES_SOLDE %d Aucune op�ration sur ce compte.", liste_clients[i].compte[compte].montant);
                if (send(sock, solde, 200, 0) != 200) {
                    fclose(bdd_clients);
                    Die("Failed to send bytes to client");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fclose(bdd_clients);
                    Die("Failed to receive additional bytes from client");
                }

            }

            else if (!strcmp("OPERATIONS", requete)) {
                char temp[BUFFSIZE];
                char message[BUFFSIZE];
                sprintf(message, "RES_OPERATIONS\n");
                int j = 0;
                while (strcmp(liste_clients[i].compte[compte].liste[j].type, "")) {
                    sprintf(temp, "%s %s %d\n", liste_clients[i].compte[compte].liste[j].type, liste_clients[i].compte[compte].liste[j].time, liste_clients[i].compte[compte].liste[j].montant);
                    strcat(message, temp);
                    j++;
                }
                if (send(sock, message, BUFFSIZE, 0) != BUFFSIZE) {
                    fclose(bdd_clients);
                    Die("Failed to send bytes to client");
                }
                if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                    fclose(bdd_clients);
                    Die("Failed to receive additional bytes from client");
                }

            }
            /* Send back received data */
            /*if (send(sock, buffer, received, 0) != received) {
            Die("Failed to send bytes to client");
            }*/
            //Check for more data
            if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
                fclose(bdd_clients);
                Die("Failed to receive additional bytes from client");
            }
        }
    }
    close(sock);
}

int main(int argc, char* argv[]) {

    //initialisation clients de la banque par lecture dans un fichier
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
            if (!strcmp(line,"\n")) continue;
            char* client = strtok(line, "  ,");
            char* password = strtok(NULL, "  ,");
            liste_clients[i].id_client = (char*) malloc(strlen(client) + 1);
            liste_clients[i].password = (char*) malloc(strlen(password) + 1);
            if (client) strcpy(liste_clients[i].id_client,client);
            if (password) strcpy(liste_clients[i].password,password);
            int nb_compte = strtol(strtok(NULL, "  ,"), NULL, 10);
            liste_clients[i].compte = (Compte*) malloc(nb_compte);
            for (int j = 0; j < nb_compte; j++) {
                char * p = strtok(NULL, "  { , }");
                if (p) liste_clients[i].compte[j].montant = strtol(p, NULL, 10);
            }
            fprintf(stdout,"%s\t",liste_clients[0].id_client);
            i++;
            nb_clients++;
        }
    }
    fclose(bdd_clients);
    int serversock, clientsock;
    struct sockaddr_in echoserver, echoclient;

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
/* Run until cancelled */
    while (1) {
        unsigned int clientlen = sizeof(echoclient);
        /* Wait for client connection */
        if ((clientsock = accept(serversock, (struct sockaddr *) &echoclient, &clientlen)) < 0) {
        Die("Failed to accept client connection");
        }
        fprintf(stdout, "Client connected: %s\n",
                        inet_ntoa(echoclient.sin_addr));
        HandleClient(clientsock);
    }
}
