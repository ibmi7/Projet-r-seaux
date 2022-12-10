#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#define BUFFSIZE 2000

int sock;
void Die(char *mess) { perror(mess); exit(EXIT_FAILURE); }
void INThandler(int sig) {
    signal(sig, SIG_IGN);
    if (sock) close(sock);
    printf("\nGoodbye\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, INThandler);
    struct sockaddr_in echoserver;
    char buffer[BUFFSIZE];
    unsigned int echolen;
    int received = 0;

    if (argc != 4) {
        fprintf(stderr, "USAGE: TCPecho <server_ip> <word> <port>\n");
        exit(EXIT_FAILURE);
    }
    /* Create the TCP socket */
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        Die("Failed to create socket");
    }

/* Construct the server sockaddr_in structure */
    memset(&echoserver, 0, sizeof(echoserver));       /* Clear struct */
    echoserver.sin_family = AF_INET;                  /* Internet/IP */
    echoserver.sin_addr.s_addr = inet_addr(argv[1]);  /* IP address */
    echoserver.sin_port = htons(atoi(argv[3]));       /* server port */
    /* Establish connection */
    if (connect(sock, (struct sockaddr *) &echoserver,sizeof(echoserver)) < 0) {
        Die("Failed to connect with server");
    }
/* Send the word to the server */
    echolen = strlen(argv[2]);
    socklen_t server_struct_length = sizeof(echoserver);
    if (sendto(sock,argv[2],echolen,0,(struct sockaddr *) &echoserver,server_struct_length)<0){
        Die("Mismatch in number of sent bytes");
    }
    /* Receive the word back from the server */
    fprintf(stdout, "Received: ");
    while (received < 3) {
        int bytes = recvfrom(sock,buffer,BUFFSIZE-1,0,(struct sockaddr *) &echoserver,&server_struct_length);
        if (bytes <1){
            Die("Failed to receive bytes from server");
        }
        received += bytes;
        buffer[bytes] = '\0';        /* Assure null terminated string */
        fprintf(stdout,"%s", buffer);
    }
    fprintf(stdout, "\n");
    close(sock);
    exit(EXIT_SUCCESS);
}
