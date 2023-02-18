#ifndef __SUBSCRIPTION__
#define __SUBSCRIPTION__

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
class Client;
#include "client.hpp"
/**
 * @brief reprezinta un abonament
 * 
 */
struct subscriptionT{
	// pointer la client
	Client *client;
	// tipul abonamentului
	bool SF;
};
/**
 * @brief reprezinta un grup de abonamente la un topic
 * 
 */
class SubscriptionGroup{
	public:
		// topic-ul grupului
		topicT topic;
		// multimea de abonamente
		// cheie = id-ul clientului
		// valoare = abonamentul
		std::map<ID_T, subscriptionT> subscribers;
		/**
		 * @brief Construct a new Subscription Group object
		 * 
		 * @param topic topicul noului grup 
		 */
		SubscriptionGroup(topicT topic);
		/**
		 * @brief adauga un nou abonament 
		 * 
		 * @param subscription 
		 */
		void receive_subscription(subscriptionT subscription);
		/**
		 * @brief inlatura abonamentul clientului cu id-ul dat
		 * 
		 * @param id 
		 */
		void remove_subscription(ID_T id);
		/**
		 * @brief trimite un mesaj tuturor clientilor abonati
		 * 
		 * @param message 
		 * @return 1 - toti clientii abonati au primit mesajul
		 * 		  -1 - una dintre transmisiuni a esuat 
		 */
		int broadcast_message(UDPmessage &message);

};

#endif