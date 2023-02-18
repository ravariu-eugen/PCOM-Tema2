#include "client.hpp"

Client::Client(ID_T id, int socket){
	this->id = id;
	this->socket = socket;
}

int Client::connect(int socket){
	if(socket < 0)
		return -2;
	this->socket = socket;
	while(!messageQueue.empty()) { // goleste coada
		UDPmessage msg = messageQueue.front();
		messageQueue.pop();
		// odata conectat, receive_message nu va mai pune pe coada
		int r = receive_message(msg);
		if(r == 2) break; // pune pe coada, nu ar trebui sa aiba loc
		if(r < 0) return r;
	}
	return 1;
}
void Client::disconnect(){
	socket = -1;
}

bool Client::isConnected(){
	return socket != -1;
}

void Client::subscribe(SubscriptionGroup *group, bool SF){
	subscriptionT subscription;
	subscription.client = this;
	subscription.SF = SF;
	group->receive_subscription(subscription);
}

void Client::unsubscribe(SubscriptionGroup *group){
	group->remove_subscription(id);
}

void Client::subscribe_to_topic(topicT topic, bool SF, std::map<topicT, SubscriptionGroup*> &groups){
	auto it = groups.find(topic);
	SubscriptionGroup *topic_group;
	if(it == groups.end()){
		topic_group = new SubscriptionGroup(topic);
		groups.insert({topic, topic_group});
	}
	else topic_group = it->second;
	subscribe(topic_group, SF);
}

void Client::unsubscribe_from_topic(topicT topic, std::map<topicT, SubscriptionGroup*> &groups){
	auto it = groups.find(topic);
	// cauta daca exista un grup cu topicul dat in multimea de grupe
	if(it != groups.end())	
		unsubscribe(it->second);
}

int Client::receive_message(UDPmessage &message){
	if(!isConnected()){
		messageQueue.push(message);
		return 2;
	}
	else return send_data(socket, (char *)(&message), message.len);
}
