#include "subscriptionGroup.hpp"

SubscriptionGroup::SubscriptionGroup(topicT topic){
	this->topic = topic;
}
void SubscriptionGroup::receive_subscription(subscriptionT subscription){
	subscribers.insert({subscription.client->id, subscription});
}
void SubscriptionGroup::remove_subscription(ID_T id){
	subscribers.erase(id);
}
int SubscriptionGroup::broadcast_message(UDPmessage &message){
    int r;
	for(auto it = subscribers.begin(); it != subscribers.end(); it++){
		subscriptionT subscription = it->second;
		Client &client = *(subscription.client);
		if(client.socket != -1 || subscription.SF == 1){
			r = client.receive_message(message);
            if(r == -1) return -1;
		}
	}
    return 1;
}