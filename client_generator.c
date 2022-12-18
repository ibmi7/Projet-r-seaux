#include <stdio.h>
#include <stdlib.h>
#include "client.h"
#include <string.h>
#include <time.h>
#include <unistd.h>

//fonction qui génère un string aléatoire de taille size
char* random_string(int size)
{
    char* string = malloc(size);
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    if (size) {
        --size;
        for (int n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            string[n] = charset[key];
        }
        string[size] = '\0';
    }
    return string;
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    FILE* fichier = NULL;
    fichier = fopen("bdd_clients_generated.csv", "w");
    fprintf(fichier,"id_client;password;nb_comptes;solde\n");
    int i;
    int nb_clients;
    if (argc != 2) nb_clients = 1000;
    else nb_clients = atoi(argv[1]);
    for (i = 0; i < nb_clients; ++i)
    {
        int nb_comptes = rand()%(6)+1;
        fprintf(fichier,"%s;%s;%d;",random_string(rand()%(6)+5),random_string(rand()%(9)+8),nb_comptes);
        int j;
        for (j = 0; j < nb_comptes-1; ++j)
        {
            fprintf(fichier,"%d;",rand()%100000);
        }
        fprintf(fichier,"%d\n",rand()%100000);
    }
    fclose(fichier);
    return 0;
}
