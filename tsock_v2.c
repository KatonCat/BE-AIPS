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
void sourceUDP(int port, char* msg, int lg_msg, int nb_msg); 

void puitTCP(int port, int lg_msg, int nb_msg); 
void sourceTCP(int port, char* msg, int lg_msg, int nb_msg); 


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
		

		// udp source
		destination = argv[argc-2];
		if (proto == 0) {
				if (nb_message== -1) nb_message=10; 
				sourceUDP(port,destination, lg_msgs, nb_message); 
			} 
		else if (proto==1)
		{
			if (nb_message== -1) nb_message=10; 
			sourceUDP(port,destination, lg_msgs, nb_message); 
		}
	} 
	else {
			printf("on est dans le puits\n");
		// udp puits

			if (proto == 0) {
				puitUDP(port, lg_msgs, nb_message); 
			}
			else if (proto == 1){
				puitTCP(port, lg_msgs, nb_message);
			}

	}
		

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
	if (nb_msg != -1) {
		for (int i=0; i < nb_msg; i++)
		{
			recvfrom(socklocal, msg,lg_msg,0,(struct sockaddr *)&addr_em,&lg_adr_em);
			afficher_message(msg, lg_msg);
		} 
	} else {

		while(1) {
			recvfrom(socklocal, msg,lg_msg,0,(struct sockaddr *)&addr_em,&lg_adr_em);
			afficher_message(msg, lg_msg);
		}
	}
	/* Puits */
	close(socklocal); 
	
}

void sourceUDP(int port, char* host, int lg_msg, int nb_msg)
{
	struct hostent *hp;
	int socklocal = socket(AF_INET,SOCK_DGRAM,0);

	struct sockaddr_in addr_serv; 
	memset((char *)&addr_serv, 0, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = htons(port);
	

	if ((hp = gethostbyname(host)) == NULL)
	{
		printf("Erreur gethostbyname\n");
		exit(1);
	}
	memcpy((char*)&(addr_serv.sin_addr),
			hp->h_addr,
			hp->h_length);

	char * msg = malloc(sizeof(char)*lg_msg);

	int lg_adr_serv = sizeof(addr_serv);
	int octets_envoyes; 
	for (int i=0; i < nb_msg; i++)
	{
		construire_message(msg,'a', lg_msg); 
		octets_envoyes=sendto(socklocal, msg,lg_msg,0,(struct sockaddr *)&addr_serv,lg_adr_serv);
		printf("envoi %d\n", octets_envoyes); 
		afficher_message(msg, lg_msg);
	} 
	close(socklocal);

}


void puitTCP(int port, int lg_msg, int nb_msg)
{
	printf("On est bien dans TCP\n");
	/* Création socket udp (source & puits) */
	int socklocal = socket(AF_INET,SOCK_STREAM,0);
	int sock_bis;
	/* reset */
    
	/* Configuration : sockaddr_in */
	struct sockaddr_in adr_client;
	memset((char*)& adr_client,0,sizeof(adr_client)); 
	adr_client.sin_family = AF_INET;
	adr_client.sin_port = htons(port);
	adr_client.sin_addr.s_addr= INADDR_ANY;
	int lg_adr = sizeof(adr_client);
	int lg_rec;
	int max=10;
	int lg_max = 30;


	/* puits ==> réception & source ==> envoie*/
	bind(socklocal,(struct sockaddr*)&adr_client, lg_adr);
	char * msg;
	msg = malloc(lg_msg);

	listen(socklocal, 5);

	if ((sock_bis = accept( socklocal,
							(struct sockaddr*)&adr_client,
							&lg_adr)) == -1 )
	{
		printf("échec du accept\n");
		exit(1);
	}


		

	

	if (nb_msg != -1) {
		for (int i=0; i < nb_msg; i++)
		{
			if((lg_rec = read(sock_bis, msg, lg_max)) < 0)
			{
			printf("échec du read\n");
			exit(1);
			}
			afficher_message(msg, lg_rec);
			if(lg_rec==0) //arrêt de l'affichage si toutes les données envoyées ont bien été reçues et que les affichages sont vides
                break;
    
		} 
	} else {
		while(lg_rec !=0) {


			if((lg_rec = read(sock_bis, msg, lg_max)) < 0)
			{
			printf("échec du read\n");
			exit(1);
			}

			afficher_message(msg, lg_rec);
	
		}
	}
	/* Puits */
	printf("Fin\n");
	free(msg);
	close(socklocal);
	
	
}



void sourceTCP(int port, char* host, int lg_msg, int nb_msg)
{
	struct hostent *hp;
	int socklocal = socket(AF_INET,SOCK_STREAM,0);
	int sock_bis;

	struct sockaddr_in addr_serv; 
	memset((char *)&addr_serv, 0, sizeof(addr_serv));
	addr_serv.sin_family = AF_INET;
	addr_serv.sin_port = htons(port);

	int lg_adr_serv = sizeof(addr_serv);

	if((sock_bis = connect(socklocal,(struct sockaddr*)&addr_serv,lg_adr_serv))==-1)
	{
		printf("échec du connect\n");
		exit(1);
	}

	if ((hp = gethostbyname(host)) == NULL)
	{
		printf("Erreur gethostbyname\n");
		exit(1);
	}

	memcpy((char*)&(addr_serv.sin_addr),
			hp->h_addr,
			hp->h_length);

	char * msg = malloc(sizeof(char)*lg_msg);


	int octets_envoyes; 
	for (int i=0; i < nb_msg; i++)
	{	

		construire_message(msg,'a', lg_msg);
		if((octets_envoyes=write(sock_bis, msg,lg_msg)) < 0)
			{
			printf("échec du write\n");
			exit(1);
			}
		
		printf("envoi %d\n", octets_envoyes); 
		afficher_message(msg, lg_msg);
	} 

	free(msg);
	close(socklocal);
	
	
}

