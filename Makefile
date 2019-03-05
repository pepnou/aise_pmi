CC=gcc
CFLAGS=-Wall -W -O3 -g
LFLAGS=-c
TARGET=libpmi.so pmiserver test_value test_perf test_perso
OBJ=./server/main.o ./hashtab/hashtab.o ./key/key.o ./queue/queue.o ./safeIO/safeIO.o ./sha-256/sha-256.o

all: $(TARGET)

libpmi.so: ./libpmi/pmi.c
	$(CC) $(CFLAGS) -shared -fpic $^ -o $@

pmiserver: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)
	
%.o: %.c %.h
	$(CC) $(LFLAGS) $^


test_value: libpmi.so test/test_values.c
	$(CC) $(CFLAGS) -Wl,-rpath,$(PWD) -L. -lpmi -o $@ -I$(PWD)/libpmi/ $^


test_perf: libpmi.so test/test_perf.c
	$(CC) $(CFLAGS) -Wl,-rpath,$(PWD) -L. -lpmi -o $@ -I$(PWD)/libpmi/ $^


test_perso: libpmi.so test/test_perso.c
	$(CC) $(CFLAGS) -Wl,-rpath,$(PWD) -L. -lpmi -o $@ -I$(PWD)/libpmi/ $^	


clean:
	rm -fr $(TARGET) $(OBJ)


