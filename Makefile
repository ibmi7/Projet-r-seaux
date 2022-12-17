all: tcp udp

tcp: tcp_client tcp_serveur

udp: udp_client udp_serveur

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

