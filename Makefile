CC=g++
CFLAGS=-g -I$(CZ_DIR)/include
_SOURCE_DIR=src
SOURCE_FILES=emoji_zwj.c
CZ_DIR=/home/blazingkin/Documents/Code/cz
LD_FLAGS=-lcz -L$(CZ_DIR)/build/release

all: svg.o emoji_zwj.o
	$(CC) $(LD_FLAGS)   -o emoji_zwj bin/svg.o bin/emoji_zwj.o

svg.o:
	$(CC) -c $(CFLAGS) -o bin/svg.o src/svg.cc

emoji_zwj.o:
	$(CC) -c $(CFLAGS) -o bin/emoji_zwj.o src/emoji_zwj.cc
