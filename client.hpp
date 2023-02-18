#ifndef __CLIENT__
#define __CLIENT__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "helpers.hpp"
#include <map>
#include <queue>
class SubscriptionGroup;
#include "subscriptionGroup.hpp"


/**
 * @brief reprezinta un client al serverului
 * 
 */
class Client{
	public:
		// id-ul clientului
		ID_T id;
		// socketul ocupat de client
		// daca socket == -1, atunci clientul este deconectat
		int socket;
		// coada de mesaje pentru abonamentele cu SF
		std::queue<UDPmessage> messageQueue;
		// topic-urile la care este abonat clientul
		std::map<topicT, SubscriptionGroup *> subscriptions;

		/**
		 * @brief Construct a new Client object
		 * 
		 * @param id 
		 * @param socket 
		 */
		Client(ID_T id, int socket);
		/**
		 * @brief conecteaza clientul la socketul dat
		 * 		  daca are mesaje pe coada, le trimite pe toate
		 * 
		 * @param socket 
		 * @return int 1 - succes
		 * 			  -1 - eroare la transmisie
		 * 			  -2 - socket invalid
		 */
		int connect(int socket);
		/**
		 * @brief deconecteaza clientul
		 * 
		 */
		void disconnect();
		/**
		 * @brief verifica daca clientul este conectat
		 * 
		 * @return true - e conectat
		 * @return false - e deconectat
		 */
		bool isConnected();
		/**
		 * @brief aboneaza clientul la un subscriptionGroup
		 * 
		 * @param group 
		 * @param SF tipul de abonament
		 */
		void subscribe(SubscriptionGroup *group, bool SF);
		/**
		 * @brief dezaboneaza clientul de la subscriptionGroup
		 * 
		 * @param group 
		 */
		void unsubscribe(SubscriptionGroup *group);
		/**
		 * @brief aboneaza clientul la grupul cu topicul dat
		 * 
		 * @param topic 
		 * @param SF tipul de abonament
		 * @param groups multimea de grupuri
		 */
		void subscribe_to_topic(topicT topic, bool SF, std::map<topicT, SubscriptionGroup *> &groups);
		/**
		 * @brief dezaboneaza clientul de la grupul cu topicul dat,
		 * sau creaza un grup cu topicul dat
		 * 
		 * @param topic 
		 * @param groups 
		 */
		void unsubscribe_from_topic(topicT topic, std::map<topicT, SubscriptionGroup *> &groups);
		/**
		 * @brief trimite mesajul pe socket daca e conectat; 
		 * 		  altfel, il pune pe coada.
		 * 
		 * @param message 
		 * @return 2 - mesaj pus pe coada
		 * 		   1 - trimitere cu succes
		 * 		  -1 - esec la transmitere 
		 */
		int receive_message(UDPmessage &message);
};



#endif