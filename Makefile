CC=g++ -std=c++11
ZMQ_PATH=/home/jose/zmq
ZMQ_INIC=$(ZMQ_PATH)/include
ZMQ_LIB=$(ZMQ_PATH)/lib
LIBS=-lzmq -lzmqpp -lpthread -lsfml-audio

all: client server

client:client.cpp
	$(CC) -L$(ZMQ_LIB) -I$(ZMQ_INIC) client.cpp -o client $(LIBS)

server:server.cpp
	$(CC) -L$(ZMQ_LIB) -I$(ZMQ_INIC) server.cpp -o server $(LIBS)
