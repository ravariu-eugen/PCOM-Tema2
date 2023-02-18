# Protocoale de comunicatii:
# Laborator 8: Multiplexare
# Makefile


# Portul pe care asculta serverul (de completat)
PORT = 1234

# Adresa IP a serverului (de completat)
IP_SERVER = 127.0.0.1


CC = g++
CFLAGS = -Wall -Wextra -std=c++17 -Werror=vla
LDFLAGS = -lm

SRCS = $(sort $(wildcard *.cpp))
TARGETS = server subscriber

# build all targets
all: $(TARGETS)

# general rule for building a target


# Compileaza server.c
server: server.cpp helpers.cpp client.cpp subscriptionGroup.cpp
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compileaza client.c
subscriber: subscriber.cpp helpers.cpp
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean run_server run_client

# Ruleaza serverul
run_server: server
	./server ${PORT} 

# Ruleaza clientul
run_subscriber_1: subscriber
	./subscriber subs1 ${IP_SERVER} ${PORT}

run_subscriber_2: subscriber
	./subscriber subs2 ${IP_SERVER} ${PORT}

run_subscriber_3: subscriber
	./subscriber subs3 ${IP_SERVER} ${PORT}

clean:
	rm -f *.o $(TARGETS)
