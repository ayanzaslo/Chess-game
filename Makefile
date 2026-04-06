CC = gcc
CFLAGS = -Wall -Wextra -std=c11

OBJS = main.o tabla.o reguli.o io.o
TARGET = sah

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c tabla.h reguli.h io.h
	$(CC) $(CFLAGS) -c main.c

tabla.o: tabla.c tabla.h
	$(CC) $(CFLAGS) -c tabla.c

reguli.o: reguli.c reguli.h tabla.h
	$(CC) $(CFLAGS) -c reguli.c

io.o: io.c io.h tabla.h
	$(CC) $(CFLAGS) -c io.c

clean:
	rm -f $(OBJS) $(TARGET)
