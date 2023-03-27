CC=gcc
CFLAGS=-std=c99 -Wall -Wextra
LOAD=load_balancer
SERVER=server
HT=hashtable
LL=linkedlist
OCDLL=ocdll

.PHONY: build clean

build: loadbalancer

loadbalancer: main.o  $(LL).o $(HT).o $(OCDLL).o $(LOAD).o $(SERVER).o
	$(CC) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) $^ -c

$(HT).o: $(HT).c $(HT).h
	$(CC) $(CFLAGS) $^ -c

$(LL).o: $(LL).c $(LL).h
	$(CC) $(CFLAGS) $^ -c

$(SERVER).o: $(SERVER).c $(SERVER).h
	$(CC) $(CFLAGS) $^ -c

$(LOAD).o: $(LOAD).c $(LOAD).h
	$(CC) $(CFLAGS) $^ -c

$(OCDLL).o: $(OCDLL).c $(OCDLL).h
	$(CC) $(CFLAGS) $^ -c

clean:
	rm -f *.o loadbalancer *.h.gch
