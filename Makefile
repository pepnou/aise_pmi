CC=gcc
CFLAGS=-Wall -W -O0 -g
LFLAGS= -L. -lpmi
TARGET=libpmi.so pmiserver test_value test_perf test_perso
OBJ=./server/main.o ./hashtab/hashtab.o ./key/key.o ./queue/queue.o ./safeIO/safeIO.o ./sha256/sha256.o

all: $(TARGET)

libpmi.so: ./libpmi/pmi.c ./safeIO/safeIO.c ./key/key.c ./sha256/sha256.c
	$(CC) $(CFLAGS) -shared -fPIC $^ -o $@

pmiserver: $(OBJ)
	$(CC) $(CFLAGS) -pg -o $@ $(OBJ)
	
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@


test_value: libpmi.so test/test_values.c
	$(CC) $(CFLAGS) -Wl,-rpath,$(PWD) -o $@ -I$(PWD)/libpmi/ $^ $(LFLAGS)


test_perf: libpmi.so test/test_perf.c
	$(CC) $(CFLAGS) -Wl,-rpath,$(PWD) -o $@ -I$(PWD)/libpmi/ $^ $(LFLAGS)


test_perso: libpmi.so test/test_perso.c
	$(CC) $(CFLAGS) -Wl,-rpath,$(PWD) -o $@ -I$(PWD)/libpmi/ $^ $(LFLAGS)


clean:
	rm -fr $(TARGET) $(OBJ)


