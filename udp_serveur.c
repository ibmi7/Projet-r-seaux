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
#define MAXPENDING 5    /* Max connection requests */
#define BUFFSIZE 1000

int serversock, clientsock;
struct sockaddr_in echoserver, echoclient;
time_t rawtime;
struct tm* timeinfo;
int nb_clients = 0;

void Die(char *mess) { perror(mess); exit(EXIT_FAILURE); }

Client liste_clients[2];
char liste_operations[5][10][BUFFSIZE];
int op[5] = {0,0,0,0,0};

void HandleClient(char* buffer) {

    char*requete = NULL;
    char*client = NULL;
    char*password = NULL;
    char* pend = NULL;
    int compte = -1;

    requete = strtok(buffer, " \t");
    /* Send bytes and check for more incoming data in loop */
    //r�cup�ration id_client
    client = strtok(NULL, " \t");
    if (!client) {
        if (sendto(serversock,"KO",3,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
            Die("Failed to send bytes to client");
        }
        fprintf(stderr,"Identifiant invalide.");
        return;
    }

    //r�cup�ration num�ro de compte
    char* temp = strtok(NULL, " \t");
    if (temp) compte = strtol(temp, NULL, 10);
    else {
        if (sendto(serversock,"KO",3,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
            Die("Failed to send bytes to client");
        }
        fprintf(stderr,"Numéro de compte invalide.");
    }
    if (compte<0 || compte >5) {
        fprintf(stderr, "compte\n");
        if (sendto(serversock,"KO",3,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
            Die("Failed to send bytes to client");
        }
        fprintf(stderr,"Num�ro de compte invalide.");
        return;
    }

    //r�cup�ration mot de passe
    password = strtok(NULL, " \t");
    if (!password) {
        fprintf(stderr, "compte\n");
        if (sendto(serversock,"KO",3,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
            Die("Failed to send bytes to client");
        }
        fprintf(stderr,"Mot de passe invalide.");
        return;
    }

    int i = 0;
    //on v�rifie que le client est bien dans la base de donn�e
    while (i < nb_clients && strcmp(liste_clients[i].id_client, client)) i++;
    if (i >= nb_clients) {
        if (sendto(serversock,"KO",3,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
            Die("Failed to send bytes to client");
        }
        return;
    }
    //on v�rifie que le mdp correspond bien
    if (strcmp(liste_clients[i].password, password)) {
        if (sendto(serversock,"KO",3,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
            Die("Failed to send bytes to client");
        }
        return;
    }


    //on verifie quelle operation
    if (!strcmp("AJOUT", requete)) {

        //on r�cup�re la somme � ajouter et on v�rifie que le num�ro de compte est valide
        temp = strtok(NULL, " ");
        int somme = 0;
        if (temp) somme = strtol(temp, &pend, 10);
        else {
            if (sendto(serversock,"KO",3,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
                Die("Failed to send bytes to client");
            }
            return;
        }
        if (compte<liste_clients[i].nb_compte) liste_clients[i].compte[compte].montant += somme;
        else{
            fprintf(stderr, "compte\n");
            if (sendto(serversock,"KO",3,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
                Die("Failed to send bytes to client");
            }
            fprintf(stderr,"Num�ro de compte invalide.");
            return;
        }

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

        if (sendto(serversock,"OK",3,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
            Die("Failed to send bytes to client");
        }
    }

    else if (!strcmp("RETRAIT", requete)) {

        //on r�cup�re la somme � retirer et on v�rifie que le num�ro de compte est valide
        temp = strtok(NULL, " ");
        int somme = 0;
        if (temp) somme = strtol(temp, &pend, 10);
        else {
            if (sendto(serversock,"KO",3,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
                Die("Failed to send bytes to client");
            }
            return;
        }
        if (compte<liste_clients[i].nb_compte) liste_clients[i].compte[compte].montant -= somme;
        else{
            fprintf(stderr, "compte\n");
            if (sendto(serversock,"KO",3,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
                Die("Failed to send bytes to client");
            }
            fprintf(stderr,"Num�ro de compte invalide.");
            return;
        }

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

        if (sendto(serversock,"OK",3,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
            Die("Failed to send bytes to client");
        }
    }

    else if (!strcmp("SOLDE", requete)) {
        char solde[200];
        if (compte>=liste_clients[i].nb_compte){
            fprintf(stderr, "compte\n");
            if (sendto(serversock,"KO",3,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
                Die("Failed to send bytes to client");
            }
            fprintf(stderr,"Num�ro de compte invalide.");
            return;
        }

        if (strcmp(liste_clients[i].compte[compte].liste[op[compte]-1].time, "")) sprintf(solde, "RES_SOLDE %d %s\n",liste_clients[i].compte[compte].montant, liste_clients[i].compte[compte].liste[op[compte]-1].time);
        else sprintf(solde, "RES_SOLDE %d Aucune op�ration sur ce compte.",liste_clients[i].compte[compte].montant);
        if (sendto(serversock,solde,200,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
            Die("Failed to send bytes to client");
        }

    }

    else if (!strcmp("OPERATIONS", requete)) {
        char temp[BUFFSIZE];
        char message[BUFFSIZE];
        sprintf(message, "RES_OPERATIONS\n");
        int j = 0;
        while (j<op[compte]){
            sprintf(temp, "%s %s %d\n", liste_clients[i].compte[compte].liste[j].type, liste_clients[i].compte[compte].liste[j].time, liste_clients[i].compte[compte].liste[j].montant);
            strcat(message, temp);
            j++;
        }
        if (sendto(serversock,message,BUFFSIZE,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
            Die("Failed to send bytes to client");
        }
        
    }

    else if (!strcmp("exit", requete)) {
        if (sendto(serversock,"OK",3,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
            Die("Failed to send bytes to client");
        }
        return;
    }

        else {
            if (sendto(serversock,"KO",3,0,(struct sockaddr*)&echoclient,sizeof(echoclient))<0) {
                Die("Failed to send bytes to client");
            }
            fprintf(stderr, "Requete invalide.");
            return;
        }
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
    FILE* bdd_clients = fopen("bdd_clients.csv", "r+");
    char* line = NULL;
    size_t len = 0;
    int i = 0;
    if (bdd_clients == NULL) {
        bdd_clients = fopen("bdd_clients.csv", "w");
        fprintf(bdd_clients, "id_client;password;nb_comptes;solde");
    }
    else {
        getline(&line, &len, bdd_clients);
        while (getline(&line, &len, bdd_clients) != -1){
            if (!strcmp(line,"\n")) continue;
            char* client = strtok(line, "  , ;");
            char* password = strtok(NULL, "  , ;");
            liste_clients[i].id_client = (char*) malloc(strlen(client) + 1);
            liste_clients[i].password = (char*) malloc(strlen(password) + 1);
            if (client) strcpy(liste_clients[i].id_client,client);
            if (password) strcpy(liste_clients[i].password,password);
            int nb_compte = strtol(strtok(NULL, "  , ;"), NULL, 10);
            liste_clients[i].compte = (Compte*) malloc(nb_compte);
            liste_clients[i].nb_compte = nb_compte;
            for (int j = 0; j < nb_compte; j++) {
                char * p = strtok(NULL, "  { , } ;");
                if (p) liste_clients[i].compte[j].montant = strtol(p, NULL, 10);
            }
            i++;
            nb_clients++;
        }
    }
    fclose(bdd_clients);

    if (argc != 2) {
        fprintf(stderr, "USAGE: echoserver <port>\n");
        exit(EXIT_FAILURE);
    }
    /* Create the UDP socket */
    if ((serversock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
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
    /*if (listen(serversock, MAXPENDING) < 0) {
    Die("Failed to listen on server socket");
    }*/
/* Run until cancelled */
    while (1) {
        //unsigned int clientlen = sizeof(echoclient);
        /* Wait for client connection */
        /*if ((clientsock = accept(serversock, (struct sockaddr *) &echoclient, &clientlen)) < 0) {
        Die("Failed to accept client connection");
        }
        fprintf(stdout, "Client connected: %s\n",
                        inet_ntoa(echoclient.sin_addr));*/
        char buffer[50];
        int received = -1;
        socklen_t client_struct_length = sizeof(echoclient);
        /* Receive message */
        if ((received = recvfrom(serversock, buffer, 50, 0,(struct sockaddr*)&echoclient, &client_struct_length)) < 0) {
            Die("Failed to receive initial bytes from client");
        }
        buffer[received] = '\0';
        if (!strcmp(buffer,"exit")) continue;
        HandleClient(buffer);
    }
}
