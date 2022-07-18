OBJS += syscall.o main.o
TARGET ?= mini
CC := gcc
CFLAGS =
CFLAGS += -g
CFLAGS += -Wall

.SUFFIXES: .c .o

all :		$(TARGET)

$(TARGET) :	$(OBJS)
		$(CC) $(OBJS) -o $(TARGET) $(CFLAGS)

.c.o :		$<
		$(CC) -c $< $(CFLAGS)

clean :
		rm -f $(OBJS) $(TARGET)