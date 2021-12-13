TARGET = UnixLs
OBJS = UnixLs.o

CFLAGS = -Wall -g -std=c99 -D_POSIX_C_SOURCE=200112L
CC = gcc

all: clean $(TARGET)

%.o : %.c
	$(CC) -c $(CFLAGS) $<

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

clean:
	rm -f $(TARGET)
	rm -f $(OBJS)
