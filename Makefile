CC=g++
CFLAGS=-g -Wall -c11 -pedantic
_SOURCE_DIR=src
SOURCE_FILES=emoji_zwj.c
CZ_DIR=/home/blazingkin/Documents/Code/cz

all:
	$(CC) -I$(CZ_DIR)/include -L$(CZ_DIR)/build/debug -lcz -o emoji_zwj src/svg.cc src/emoji_zwj.cc