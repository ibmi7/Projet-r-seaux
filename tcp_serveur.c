#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>
#include "client.h"
#define MAXPENDING 5    /* Max connection requests */
#define BUFFSIZE 32
void Die(char *mess) { perror(mess); exit(EXIT_FAILURE); }

Client liste_clients[2];

void HandleClient(int sock) {
    char buffer[BUFFSIZE];
    int received = -1;
    /* Receive message */
    if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
        Die("Failed to receive initial bytes from client");
    }
    char*requete;
    char*client;
    char*password;
    int compte;
    requete = strtok(buffer, " ");
    /* Send bytes and check for more incoming data in loop */
    while (received > 0) {
        //on verifie quelle operation
        if (!strcmp("AJOUT", requete)) {
            //récupération id_client et password
            client = strtok(NULL, " ");
            compte = strtok(NULL, " ")[0] - 48;
            password = strtok(NULL, " ");
            int i = 0;
            while (strcmp(liste_clients[i].id_client, client) && i<2) i++;
            if (i >= 2) Die("ID client non trouvé");
            if (strcmp(liste_clients[i].password, password)) Die("Le mdp ne correspond pas");
            int somme = strtol(strtok(NULL, " "), NULL, 10);
            liste_clients[i].compte[compte] = somme;
            if (send(sock, "OK", received, 0) != received) {
                Die("Failed to send bytes to client");
            }
            //on trouve le client dans la base et on verifie que c'est le bon password
        }
        else if (!strcmp("RETRAIT", requete)) {

        }
        else if (!strcmp("SOLDE", requete)) {

        }
        else if (!strcmp("OPERATIONS", requete)) {

        }
        /* Send back received data */
        if (send(sock, buffer, received, 0) != received) {
        Die("Failed to send bytes to client");
        }
        /* Check for more data */
        if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
        Die("Failed to receive additional bytes from client");
        }
    }
    close(sock);
}

int main(int argc, char *argv[]) {
    //initialisation clients
    liste_clients[0].id_client = "ibrahim";
    liste_clients[0].password = "azerty";
    liste_clients[1].id_client = "youssef";
    liste_clients[1].password = "qwerty";
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
