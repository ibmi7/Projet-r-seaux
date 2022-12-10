

tcp: tcp_client tcp_serveur

tcp_client: tcp_client.c client.h
	gcc -Wall -g tcp_client.c -o tcp_client.out

tcp_serveur: tcp_serveur.c client.h
	gcc -Wall -g tcp_serveur.c -o tcp_serveur.out

.PHONY : clean
 clean:
	rm *.out 

