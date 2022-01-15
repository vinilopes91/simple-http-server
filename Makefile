TARGET=http-server
CC=gcc
DEBUG=-g
OPT=-O0
WARN=-Wall
PTHREAD=-pthread
FLAGS=$(DEBUG) $(PTHREAD) $(OPT) $(WARN)
SOURCE_CODE=server.c dqueue.c

install:
	$(CC) $(FLAGS) $(SOURCE_CODE) -o $(TARGET)
