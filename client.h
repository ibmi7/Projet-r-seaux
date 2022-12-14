#include <stdlib.h>
#ifndef CLIENT_H
#define CLIENT_H

typedef struct {
	int montant;
}Compte;


typedef struct{
	char *id_client;
	char *password;
	Compte *compte;
	int nb_compte;
}Client;

#endif
