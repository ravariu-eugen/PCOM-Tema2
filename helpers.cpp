#include "helpers.hpp"

// citeste un mesaj de pe sockfd

int receive_data(int sockfd, char *buf, int *len){
	
	int rt = recv(sockfd, len, sizeof(int), 0);
	if(rt <= 0) return rt;
	int bytes_remaining = *len;
	int bytes_received = 0;
    while(bytes_received < *len){
        rt = recv(sockfd, buf + bytes_received, bytes_remaining, 0);
        if(rt < 0) return rt;
		bytes_received += rt;
        bytes_remaining -= rt;
    }
    return 1;
}

// trimite un mesaj de len octeti pe sockfd
int send_data(int sockfd, char *buf, int len){
    int bytes_sent = 0;
    int bytes_remaining = len;
    
    int rt = send(sockfd, &len, sizeof(int), 0); // trimit lungimea mesajului
    if(rt <= 0) return rt;
	while (bytes_sent < len){
        rt = send(sockfd, buf + bytes_sent, bytes_remaining, 0);
		if(rt <= 0) return rt;
        bytes_sent += rt;
        bytes_remaining -= rt;
    }
    return 1;
}

void disable_nagle(int socket){
	int yes = 1;
	int ret = setsockopt(socket,
                        IPPROTO_TCP,
                        TCP_NODELAY,
                        (char *) &yes, 
                        sizeof(int));    // 1 - on, 0 - off
 	DIE(ret < 0, "socket");
}



double sciNot(uint32_t val, uint8_t exp){
	double rez = val;
	while(exp--) rez/=10;
	return rez;
}



void print_message(UDPmessage &msg, FILE *fp){
	struct sockaddr_in temp;
	temp.sin_addr.s_addr = msg.ip;
	fprintf(fp,"%s:%d - %s - ", inet_ntoa(temp.sin_addr), msg.port, msg.topic.name);
	uint8_t semn, exp;
	uint32_t val;
	uint16_t modf;
	char *value = msg.payload;
	switch(msg.tip_date){
		case UDP_INT: 
			semn = *(uint8_t *)value;
			val = ntohl(*(uint32_t *)(value+1));
			fprintf(fp, "INT - %d\n", ((int)val)*(1-2*semn));
			break;
		case UDP_SHORT_REAL:
			modf = ntohs(*(uint16_t *)value);
			fprintf(fp, "SHORT_REAL - %d.%02d\n", modf/100, modf%100);
			break;
		case UDP_FLOAT:
			semn = *(uint8_t *)value;
			val = ntohl(*(uint32_t *)(value+1));
			exp = *(uint8_t *)(value+5);
			fprintf(fp, "FLOAT - %f\n", sciNot(val, exp)*(1-2*semn));
			break;
		case UDP_STRING:
			fprintf(fp, "STRING - %s\n", value);
			break;
		default: fprintf(fp, "ERROR_TYPE\n");
	}
}