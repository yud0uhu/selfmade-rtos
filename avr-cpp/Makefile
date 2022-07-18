OBJS += syscall.o main.o
TARGET ?= minios
CC ?= gcc
CFLAGS =
#CFLAGS += -O
CFLAGS += -g
CFLAGS += -Wall
CFLAGS += -static

.SUFFIXES: .c .o

all :		$(TARGET)

$(TARGET) :	$(OBJS)
		$(CC) $(OBJS) -o $(TARGET) $(CFLAGS)

.c.o :		$<
		$(CC) -c $< $(CFLAGS)

clean :
		rm -f $(OBJS) $(TARGET)