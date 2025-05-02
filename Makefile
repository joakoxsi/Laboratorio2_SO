CC 		:= gcc
CFLAGS  := -Wall -Wextra -g

SRCS 	:= main.c
OBJS 	:= $(SRCS:.c=.o)
TARGET	:= main

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)