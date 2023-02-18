
#include "helpers.hpp"
#include <map>
#include "subscriptionGroup.hpp"
#include "client.hpp"



/**
 * @brief trimite un mesaj grupului cu topicul dat 
 * pentru a fi transmis la abonati
 * 
 * @param topic 
 * @param msg 
 * @param groups multimea de grupe
 * @return int 1 - transmisie cu success
 * 			  -1 - a avut loc o eroare la transmisie
 * 			  0 - nu exista un grup cu topicul dat
 */
int broadcast_message_to_topic(topicT topic, UDPmessage &msg, std::map<topicT, SubscriptionGroup*> &groups){
	auto it = groups.find(topic);
	if(it == groups.end()){
		return 0;
	}
	return it->second->broadcast_message(msg);
}

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}


int main(int argc, char *argv[])

{
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	int sockfd, newsockfd, portno, dgram_sockfd;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i, ret;
	socklen_t clilen;
	

	fd_set read_fds;	// multimea de citire folosita in select()
	fd_set tmp_fds;		// multime folosita temporar
	fd_set socks;		// multime pentru socketii TCP
	int fdmax;			// valoare maxima fd din multimea read_fds
	
	if (argc < 1) {
		usage(argv[0]);
	}

	// se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);
	FD_ZERO(&socks);

	// socket pentru datagrame UDP
	dgram_sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
	DIE(dgram_sockfd < 0, "socket");

	// socket pentru ascultare conexiuni TCP
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");


	disable_nagle(sockfd);
	//disable_nagle(dgram_sockfd);

	portno = atoi(argv[1]);
	DIE(portno == 0, "atoi");

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");
	ret = bind(dgram_sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr));
	DIE(ret < 0, "bind");

	ret = listen(sockfd, MAX_BACKLOG);
	DIE(ret < 0, "listen");
	
	// se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
	FD_SET(0, &read_fds); // standard input
	FD_SET(sockfd, &read_fds);
	FD_SET(dgram_sockfd, &read_fds);
	if(sockfd > dgram_sockfd) fdmax = sockfd;
	else fdmax = dgram_sockfd;
	
	std::map<topicT, SubscriptionGroup *> topic_groups; // multimea de grupuri de abonament
	// cheia - topicul grupului
	// valoarea - pointer la grup
	std::map<ID_T, Client*> clients; // multimea de clienti
	// cheia - id-ul clientului
	// valoarea - pointer la client
	std::map<int, ID_T> client_IDs; // multimea de id-uri de clienti conectati
	// cheia - socketul pe care e conectat
	// valoarea - id-ul
	while (1) {
		tmp_fds = read_fds; 
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				memset(buffer, 0, BUFLEN);
				
				if (i == 0){ // citire de la tastatura
				
					fgets(buffer, BUFLEN - 1, stdin);
					if (strncmp(buffer, "exit", 4) == 0) {
						
						close(sockfd);
						return 0;
					}
					
				} else if (i == sockfd) {
					// a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
					DIE(newsockfd < 0, "accept");
					char new_clientID[BUFLEN]  = {0};
					int len = 0;
					// primeste id-ul clientului care tocmai s-a conectats
					ret = receive_data(newsockfd, new_clientID, &len); 
					DIE(ret < 0, "recv");
					
					if(len > CLIENT_ID_LEN){
						// id prea lung
						fprintf(stderr, "bad ID \n");
						continue;
					}
					ID_T newID;
					memset(newID.id, 0, CLIENT_ID_LEN + 1);
					memcpy(newID.id, new_clientID, len);
					auto it = clients.find(newID);
					if(it != clients.end()){ // already exists
						if(it->second->isConnected()){
							// already connected
							close(newsockfd);
							fprintf(stdout, "Client %s already connected.\n", new_clientID);
							continue;
						}
						else{ // not connected
							Client *client = it->second;
							ret = client->connect(newsockfd);
							DIE(ret < 0, "Connection fail");
							client_IDs.insert({newsockfd, newID});
						}
					}
					else{ // new client
						Client *new_client = new Client(newID, newsockfd);
						clients.insert({newID, new_client});
						client_IDs.insert({newsockfd, newID});
					}

					
					// se adauga noul socket intors de accept() la multimea descriptorilor de citire
					FD_SET(newsockfd, &read_fds);
					FD_SET(newsockfd, &socks);
					disable_nagle(newsockfd);
					if (newsockfd > fdmax) 
						fdmax = newsockfd;
					printf("New client %s connected from %s:%d.\n",
							new_clientID, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));
				} else if (i == dgram_sockfd){ // mesaj de la client UDP
					char buffer[BUFLEN];
			
					int len = recvfrom(i, buffer, sizeof(buffer), 0,(struct sockaddr *)  &cli_addr, &clilen);
					UDPmessage msg;
					msg.ip = cli_addr.sin_addr.s_addr;
					msg.port = cli_addr.sin_port;
					memcpy(msg.topic.name, buffer, TOPIC_LEN);
					msg.tip_date = *(uint8_t *)(buffer + TOPIC_LEN);
					msg.len = len  +10;
					memcpy(msg.payload, buffer + TOPIC_LEN + 1, len - TOPIC_LEN - 1);

					
					ret = broadcast_message_to_topic(msg.topic, msg, topic_groups);
					DIE(ret == -1, "broadcast_message_to_topic failed");
					
				}
				else {
					
					// s-au primit date pe unul din socketii de client,
					// asa ca serverul trebuie sa le receptioneze
					
					auto it1 = client_IDs.find(i); //
					if(it1 == client_IDs.end()) { // daca nu exista un id pt. socket
						fprintf(stderr, "1no client on socket %d\n", i);
						continue;
					}
					auto it2 = clients.find(it1->second);
					if(it2 == clients.end()) { // daca nu exista un client pe socket
						fprintf(stderr, "2no client on socket %d\n", i);
						continue;
					}
					Client *client = it2->second;
					int len = 0;
					n = receive_data(i, buffer, &len);
					DIE(n < 0, "recv");
					
					if (len == 0) {
						// conexiunea s-a inchis
						printf("Client %s disconnected.\n", client_IDs[i].id);
						close(i);
						clients.find(client_IDs[i])->second->disconnect();
						client_IDs.erase(i);
						// se scoate din multimea de citire socketul inchis 
						FD_CLR(i, &read_fds);
						FD_CLR(i, &tmp_fds);
						FD_CLR(i, &socks);
					} else { // un mesaj pentru server
						struct TCPmessage *msg = (struct TCPmessage *)buffer;
						switch(msg->type) {
							case SUBSCRIBE_NO_SF:
								client->subscribe_to_topic(msg->topic, SUBSCRIBE_NO_SF, topic_groups);
								break;
							case SUBSCRIBE_SF:
								client->subscribe_to_topic(msg->topic, SUBSCRIBE_SF, topic_groups);
								break;
							case UNSUBSCRIBE:
								client->unsubscribe_from_topic(msg->topic, topic_groups);
								break;
							default:
								fprintf(stderr, "Tip necunoscut %d\n", msg->type); continue;
						}
						


					}
				}
			}
		}
	}
	close(sockfd);

	return 0;
}
