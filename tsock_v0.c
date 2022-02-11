/* librairie standard ... */
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* constantes relatives aux domaines, types et protocoles */
#include <sys/socket.h>
/* constantes et structures propres au domaine UNIX */
#include <sys/un.h>
/* constantes et structures propres au domaine INTERNET */
#include <netinet/in.h>
/* structures retournées par les fonctions de gestion de la base de
données du réseau */
#include <netdb.h>
/* pour les entrées/sorties */
#include <stdio.h>
/* pour la gestion des erreurs */
#include <errno.h>

void puitUDP(int port, int lg_msg, int nb_msg); 
void sourceUDP(int port, char* msg); 

void main (int argc, char **argv)
{
	int c;
	extern char *optarg;
	extern int optind;
	int nb_message = -1; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int source = -1 ; /* 0=puits, 1=source */
	int port = atoi(argv[argc-1]);
	int proto = 1; /* 1 pour TCP et 0  pour UDP */
	int lg_msgs = 30; 

	while ((c = getopt(argc, argv, "upn:s")) != -1) {
		switch (c) {
		case 'p':
			if (source == 1) {
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1);
			}
			source = 0;
			break;

		case 's':
			if (source == 0) {
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1) ;
			}
			source = 1;
			break;

		case 'n':
			nb_message = atoi(optarg);
			break;

		case 'u':
			proto = 0; 
			break; 
		default:
			printf("usage: cmd [-p|-s][-n ##]\n");
			break;
		}
	}

	if (source == -1) {
		printf("usage: cmd [-p|-s][-n ##]\n");
		exit(1) ;
	}

	char * destination; 
	if (source == 1) {
		printf("on est dans le source\n");
		destination = argv[argc-2]; 
	} else
		printf("on est dans le puits\n");

	if (nb_message != -1) {
		if (source == 1)
			printf("nb de tampons à envoyer : %d\n", nb_message);
		else
			printf("nb de tampons à recevoir : %d\n", nb_message);
	} else {
		if (source == 1) {
			nb_message = 10 ;
			printf("nb de tampons à envoyer = 10 par défaut\n");
		} else
		printf("nb de tampons à envoyer = infini\n");

	}

	if (proto == 0) {
		if (source == 0)
			puitUDP(port, lg_msgs, nb_message); 
	}
}

void construire_message(char *message, char motif, int lg)
{
	int i;
	for (i=0;i<lg;i++) 
		message[i] = motif;
}

void afficher_message(char *message, int lg) 
{
	int i;
	printf("message construit : ");
	for (i=0;i<lg;i++) 
		printf("%c", message[i]); printf("\n");
}

void puitUDP(int port, int lg_msg, int nb_msg)
{
	

	

	/* Création socket udp (source & puits) */
	int socklocal = socket(AF_INET,SOCK_DGRAM,0);

	

	/* Configuration : sockaddr_in */
	struct sockaddr_in structs_local;
	structs_local.sin_family = AF_INET;
	structs_local.sin_port = htons(port);
	structs_local.sin_addr.s_addr= INADDR_ANY;
	


	int lg_adr = sizeof(structs_local);
	/* puits ==> réception & source ==> envoie*/
	bind(socklocal,(struct sockaddr*)&structs_local, lg_adr);
	char * msg;
	msg = malloc(lg_msg);

	struct sockaddr_in addr_em;
	int lg_adr_em = sizeof(addr_em); 
	
	/* source */
	// construire le message
	for (int i=0; i < nb_msg; i++)
	{
		recvfrom(socklocal, msg,lg_msg,0,(struct sockaddr *)&addr_em,lg_adr_em);
		afficher_message(msg, lg_msg);
	} 
	/* Puits */
	
	
}

void sourceUDP(int port, char* msg)
{
	int socklocal = socket(AF_INET,SOCK_DGRAM,0);

	struct sockaddr_in addr_em;
	structs_local.sin_family = AF_INET;
	structs_local.sin_port = htons(port);
	structs_local.sin_addr.s_addr= INADDR_ANY;

	int lg_adr = sizeof(addr_em);
	bind(socklocal,(struct sockaddr*)&addr_em, lg_adr);
		char * msg;
	msg = malloc(lg_msg);

	for (int i=0; i < nb_msg; i++)
	{
		sendto(socklocal, msg,lg_msg,0,(struct sockaddr *)&addr_em,lg_adr_em);
		afficher_message(msg, lg_msg);
	} 

	

	//construire_message()
	//sendto(socklocal, msg, sizeof(msg), 0, (struct sockaddr_in*)&structs_local, lg_adr);

}
