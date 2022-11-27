
tcp: tcp_client tcp_serveur

tcp_client: tcp_client.c client.h
	gcc -Wall -g tcp_client.c -o tcp_client.out

tcp_serveur: tcp_serveur.c client.h
	gcc -Wall -g tcp_serveur.c -o tcp_serveur.out

udp: udp_client udp_serveur

udp_client: udp_client.c client.h
	gcc -Wall -g udp_client.c -o udp_client.out

udp_serveur: udp_serveur.c client.h
	gcc -Wall -g udp_serveur.c -o udp_serveur.out

.PHONY : clean
 clean:
	rm *.out 

