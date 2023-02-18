#ifndef _HELPERS_H
#define _HELPERS_H 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>

/*
 * Macro de verificare a erorilor
 * Exemplu:
 *     int fd = open(file_name, O_RDONLY);
 *     DIE(fd == -1, "open failed");
 */


#define DIE(assertion, call_description)	\
	do {									\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",	\
					__FILE__, __LINE__);	\
			perror(call_description);		\
			exit(EXIT_FAILURE);				\
		}									\
	} while(0)

#define CLIENT_ID_LEN 10
#define TOPIC_LEN 50

struct topicT{
	char name[TOPIC_LEN+1];
	friend bool operator <(const topicT& t1, const topicT& t2){
		return strcmp(t1.name, t2.name) < 0;
	}
};

struct ID_T{
	char id[CLIENT_ID_LEN+1];
	friend bool operator <(const ID_T& t1, const ID_T& t2){
		return strcmp(t1.id, t2.id) < 0;
	}
};
/**
 * @brief un mesaj trimis de catre un client TCP serverului;
 * 
 */
struct TCPmessage{
	ID_T clientID;
	topicT topic;
	uint8_t type;
};
// valori pentru type
#define SUBSCRIBE_NO_SF 0 
#define SUBSCRIBE_SF 1 
#define UNSUBSCRIBE 2

#define PAYLOAD_SIZE 1500
/**
 * @brief un mesaj ce contine date primite de la un client UDP, 
 * ce va fi trimis abonatilor.
 * 
 */
struct UDPmessage{
	uint32_t ip;
	uint16_t port; 
	topicT topic;
	uint8_t tip_date;
	uint16_t len;
	char payload[PAYLOAD_SIZE];
};
// valori pentru tip date
#define UDP_INT 0
#define UDP_SHORT_REAL 1
#define UDP_FLOAT 2
#define UDP_STRING 3




#define BUFLEN 2048
#define MAX_BACKLOG 1024

/**
 * @brief citeste un mesaj de pe socket
 * 
 * @param sockfd socketul de unde citeste
 * @param buf locatia unde scrie datele primite
 * @param len locatia unde scrie lungimea mesajului
 * @return int 1 - citire cu succes
 * 			   0 - s-a terminat transmisiunea
 * 			   -1 - esec la transmisiune
 */
int receive_data(int sockfd, char *buf, int *len);

/**
 * @brief scrie un mesaj la sochet
 * 
 * @param sockfd sochetul unde scrie datele
 * @param buf locatia de unde citeste datele
 * @param len lungimea secventei trimise
 * @return int 1 - scriere cu succes
 * 			   -1 - esec la transmisiune
 */
int send_data(int sockfd, char *buf, int len);
/**
 * @brief seteaza TCP_NODELAY la 1
 * 
 * @param socket 
 */
void disable_nagle(int socket);

/**
 * @brief afiseaza continutul unui mesaj UDP
 * 
 * @param msg 
 * @param fp 
 */
void print_message(UDPmessage &msg, FILE *fp);
#endif
