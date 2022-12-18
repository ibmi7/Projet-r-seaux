# Serveur de comptes bancaires TCP

Ce programme est un serveur de sockets en C qui permet à des clients de gérer leurs comptes bancaires en envoyant des requêtes via un socket TCP.

# Prérequis

Pour compiler et exécuter ce programme, vous avez besoin des headers et bibliothèques suivantes :

    stdio.h
    sys/socket.h
    arpa/inet.h
    stdlib.h
    string.h
    unistd.h
    netinet/in.h
    signal.h
    time.h
    pthread.h

# Compilation

Pour compiler le programme, utilisez la commande suivante :

gcc tcp_serveur.c -o tcp_serveur -lpthread

Ou utilisez le Makefile fournit avec le projet, en tapant la commande make ou make tcp_serveur

# Exécution

Pour exécuter le programme, utilisez la commande suivante :

./server [port]

Où port est le numéro de port sur lequel le serveur doit écouter les connexions. A noter, le port doit être compris entre 1024 et 49151.

# Utilisation

Les clients peuvent se connecter au serveur en utilisant un logiciel de socket tel que telnet ou netcat, ou en utilisant le client fourni.

Une fois connecté, le client peut envoyer les requêtes suivantes :

    AJOUT [id_client] [n_compte] [password] [montant] : ajoute l'argent spécifié sur le compte indiqué.
    RETRAIT [id_client] [n_compte] [password] [montant] : retire l'argent spécifié sur le compte indiqué.
    SOLDE [id_client] [n_compte] [password] [montant] : affiche le solde du compte indiqué.
    OPERATIONS [id_client] [n_compte] [password] [montant] : affiche les 10 dernières opérations du compte indiqué.
    exit : déconnecte le client du serveur.

# Notes

    Le serveur peut gérer plusieurs clients simultanément grâce à l'utilisation de threads.
    Les données des comptes sont stockées dans une structure de données Client, définie dans le fichier client.h.
    Le serveur enregistre également les opérations effectuées dans un fichier log, qui est créé au lancement du programme.


# Client de socket TCP

Ce programme est un client de socket en C qui envoie des requêtes à un serveur via un socket TCP et affiche les réponses reçues.

# Prérequis

Pour compiler et exécuter ce programme, vous avez besoin des headers et bibliothèques suivants :

    stdio.h
    sys/socket.h
    arpa/inet.h
    stdlib.h
    string.h
    unistd.h
    netinet/in.h
    signal.h

# Compilation

Pour compiler le programme, utilisez la commande suivante :

gcc tcp_client.c -o tcp_client

Ou utilisez le Makefile fournit avec le projet, en tapant la commande make ou make tcp_client


# Exécution

Pour exécuter le programme, utilisez la commande suivante :

./client [server_ip] [word] [port]

Où server_ip est l'adresse IP du serveur auquel vous souhaitez vous connecter, word est la requête que vous souhaitez envoyer au serveur et port est le numéro de port sur lequel le serveur écoute les connexions.

Une fois connecté, le client peut envoyer les requêtes suivantes :

    AJOUT [id_client] [n_compte] [password] [montant] : ajoute l'argent spécifié sur le compte indiqué.
    RETRAIT [id_client] [n_compte] [password] [montant] : retire l'argent spécifié sur le compte indiqué.
    SOLDE [id_client] [n_compte] [password] [montant] : affiche le solde du compte indiqué.
    OPERATIONS [id_client] [n_compte] [password] [montant] : affiche les 10 dernières opérations du compte indiqué.
    exit : déconnecte le client du serveur.

# Version UDP

Le code est aussi disponible en version UDP. L'utilisation est la même que dans la version TCP.


# Exemple d'utilisation

En localhost, pour ajouter 500€ dans le compte n°3 du client ibrahim, en TCP, il faut :
    - lancer le serveur dans un terminal avec la commande ./tcp_serveur 2000
    - lancer le client dans un autre terminal avec la commande ./tcp_client 127.0.0.1 "AJOUT ibrahim 3 azerty 500" 2000
    - le client reçoit la réponse "OK" de la part du serveur
    - le client peut continuer à envoyer des requêtes de la forme suivante : [REQUETE] [id_client] [n_compte] [password] [montant]
En UDP, il suffit de remplacer "tcp_serveur" par "udp_serveur", et "tcp_client" par "udp_client".

# Auteur

Ce programme a été écrit par Ibrahim Minthe et Youssef Skhiri.
