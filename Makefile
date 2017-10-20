CC := gcc
CCFLAGS := -Wall -Werror -O0
LDFLAGS := -lpthread

all: run_server run_client

run_server: server.o common.o map.o list.o
	$(CC) $(CCFLAGS) -o $@ $^ $(LDFLAGS)

server.o: server/server.c
	$(CC) $(CCFLAGS) -c $^

run_client: client.o common.o map.o list.o
	$(CC) $(CCFLAGS) -o $@ $^ $(LDFLAGS)

client.o: client/client.c
	$(CC) $(CCFLAGS) -c $^

common.o: common/common.c
	$(CC) $(CCFLAGS) -c $^

map.o: common/map.c
	$(CC) $(CCFLAGS) -c $^

list.o: common/list.c
	$(CC) $(CCFLAGS) -c $^


clean:
	- rm *.o
	- rm run_client
	- rm run_server