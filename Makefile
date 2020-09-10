TARGET = graphcurses
INSTALL_PATH = /usr/local/bin

SRC = $(wildcard *.c)
OBJ = $(SRC:%.c=%.o)

CC = gcc
CPPFLAGS += -Iinclude -pedantic -U__STRICT_ANSI__
CFLAGS += -Wall -std=c99 -O3
LDFLAGS += -Llib
LDLIBS += -lm -lmatheval -lncurses

CP=cp
MOVE = mv
MKDIR_P = mkdir -p
RM_DIR=rm -rf

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run:
	./$(TARGET)

install: $(TARGET)
	$(CP) $(TARGET) $(INSTALL_PATH)

clean:
	$(RM) $(OBJ) $(TARGET)
