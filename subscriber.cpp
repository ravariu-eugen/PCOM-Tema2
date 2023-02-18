#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.hpp"
#include <queue>

void usage(char *file)
{
	fprintf(stderr, "Usage: %s client_ID server_address server_port\n", file);
	exit(0);
}


/**
 * @brief creaza un mesaj
 * 
 * @param buf ce a fost citit de la tastatura
 * @param ID id-ul clientului
 * @return TCPmessage* 
 * 			null daca buf nu are formatul corect
 */
TCPmessage *create_message(char *buf, char *ID){
	char tokens[10] = " \n";
	char *t = strtok(buf, tokens);
	TCPmessage *mesaj;	
	if(strcmp(t, "subscribe") == 0){
		// mesaj de subscribe
		t = strtok(NULL, tokens);
		if(t == NULL) return NULL;
		char *topic = t;
		t = strtok(NULL, tokens);
		if(t == NULL) return NULL;
		uint8_t SF = atoi(t);
		if(SF > 1) return NULL;
		mesaj = new TCPmessage;
		strncpy(mesaj->clientID.id, ID, CLIENT_ID_LEN);
		strncpy(mesaj->topic.name, topic, TOPIC_LEN);
		mesaj->type = SF;
		return mesaj;

	}else if(strcmp(t, "unsubscribe") == 0){
		// mesaj de unsubscribe
		t = strtok(NULL, tokens);
		if(t == NULL) return NULL;
		char *topic = t;
		mesaj = new TCPmessage;
		strncpy(mesaj->clientID.id, ID, CLIENT_ID_LEN);
		strncpy(mesaj->topic.name, topic, TOPIC_LEN);
		mesaj->type = UNSUBSCRIBE;
		return mesaj;
	}else{ // mesaj invalid
		return NULL;
	}
}


int main(int argc, char *argv[])
{
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	int sockfd, ret, i;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];



	if (argc < 4) {
		usage(argv[0]);
	}
	char *client_ID = argv[1];
	if(strlen(client_ID) > CLIENT_ID_LEN){
		printf("Invalid client ID\n");
		return 1;
	}
	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	int fdmax;
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

	disable_nagle(sockfd);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");
	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");
	FD_SET(0, &read_fds); // standard input
	FD_SET(sockfd, &read_fds); // socket
	fdmax = sockfd;
	
	send_data(sockfd, client_ID, strlen(client_ID));

	while (1) {
		tmp_fds = read_fds; 
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");

		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				memset(buffer, 0, BUFLEN);
				if (i == 0) { // citire de la tastatura
					
					fgets(buffer, BUFLEN - 1, stdin);
					if (strncmp(buffer, "exit", 4) == 0) {
						close(sockfd);
						return 0;
					}
					struct TCPmessage *mesaj = create_message(buffer, client_ID);
					if(mesaj == NULL) continue;
					ret = send_data(sockfd, (char *)mesaj, sizeof(struct TCPmessage));
					DIE(ret < 0, "send");
					if(mesaj -> type == UNSUBSCRIBE)
						printf("Unsubscribed from topic.\n");
					else if(mesaj -> type == SUBSCRIBE_NO_SF || mesaj -> type == SUBSCRIBE_SF)
						printf("Subscribed to topic.\n");
					free(mesaj);
				} else { // citire de la server
					UDPmessage mesaj;
					int len = 0;
					ret = receive_data(sockfd, buffer, &len);
					DIE(ret < 0, "recv");
				
					if(len == 0) {
						// serverul s-a inchis
						close(sockfd);
						return 0;
					}
					
					mesaj = *(UDPmessage*)buffer;
					print_message(mesaj, stdout);
				}
			}
		}
	}


	
	

	return 0;
}
