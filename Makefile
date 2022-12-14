all: tcp udp thread

tcp: tcp_client tcp_serveur

udp: udp_client udp_serveur

thread: tcp_serveur_thread client.h
	gcc -Wall -g tcp_serveur_thread.c -o tcp_serveur_thread -lpthread
tcp_serveur_fork: tcp_serveur_fork.c client.h
	gcc -Wall -g tcp_serveur_fork.c -o tcp_serveur_fork

tcp_client: tcp_client.c client.h
	gcc -Wall -g tcp_client.c -o tcp_client

tcp_serveur: tcp_serveur.c client.h
	gcc -Wall -g tcp_serveur.c -o tcp_serveur

udp_client: udp_client.c client.h
	gcc -Wall -g udp_client.c -o udp_client

udp_serveur: udp_serveur.c client.h
	gcc -Wall -g udp_serveur.c -o udp_serveur

.PHONY : clean
 clean:
	rm tcp_client tcp_serveur udp_client udp_serveur

