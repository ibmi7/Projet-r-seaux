#include <stdlib.h>

typedef struct {
	char type[20];
	int montant;
	char time[100];
}Operation;

typedef struct {
	int montant;
	Operation liste[10];
}Compte;


typedef struct{
	char *id_client;
	char *password;
	Compte*compte;
	int nb_compte;
}Client;




