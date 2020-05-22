TARGET = graphcurses 
INSTALL_PATH = /usr/local/bin

SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

MOVE = mv
MKDIR_P = mkdir -p

CC = gcc
CPPFLAGS += -Iinclude -pedantic -U__STRICT_ANSI__
CFLAGS += -Wall -std=c99
LDFLAGS += -Llib
LDLIBS += -lm -lmatheval -lncurses

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(MKDIR_P) $(BIN_DIR)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
	$(MOVE) $(TARGET) $(BIN_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(MKDIR_P) $(OBJ_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run:
	./$(BIN_DIR)/$(TARGET)

install: $(TARGET)
	cp $(BIN_DIR)/$(TARGET) $(INSTALL_PATH)
	
clean:
	$(RM) $(OBJ) $(BIN_DIR)/$(TARGET)
