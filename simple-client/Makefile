CFLAGS+=-Wall -Wextra -std=gnu99

simple-client: simple-client.o

simple-client.o: simple-client.c message.h

.PHONY: clean
clean:
	rm -f simple-client.o
	rm -f simple-client
