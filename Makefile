#
# GDBFrontendLive is a server that creates sharable GDBFrontend instances and provides online IDEs.
#
# https://github.com/rohanrhu/gdb-frontend-live
# https://oguzhaneroglu.com/projects/gdb-frontend-live/
#
# Licensed under GNU/GPLv3
# Copyright (C) 2020, Oğuzhan Eroğlu (https://oguzhaneroglu.com/) <rohanrhu2@gmail.com>
#

CC = gcc

CFLAGS = -std=c99 \
		 -I. \
		 -g \
		 -luuid \
		 -lssl \
		 -lcrypto \
		 -lpthread \
		 -lm

LDFLAGS =

SOURCES = $(shell find . -wholename "./src/*.c")
HEADERS = $(shell find . -wholename "./include/*.h")
EXECUTABLES = server
OBJECTS = $(addprefix ./dist/, $(notdir $(filter-out ./src/server.o, $(SOURCES:.c=.o))))
RM = rm -rf

.PHONY: clean

all: server

dist:
	mkdir -p dist/

server: dist $(OBJECTS) jsonic
	$(CC) -o $@ src/server.c $(filter-out dist jsonic, $^) lib/jsonic/jsonic.o $(CFLAGS) $(LDFLAGS)
	chmod +x server
	$(RM) dist/
	@echo "\033[32mExecutable: ./server is built.\033[0m"

dist/util.o:
	$(CC) -c -o $@ src/util.c $(CFLAGS) $(LDFLAGS)

dist/instance.o:
	$(CC) -c -o $@ src/instance.c $(CFLAGS) $(LDFLAGS)

dist/uniqid.o:
	$(CC) -c -o $@ src/uniqid.c $(CFLAGS) $(LDFLAGS)

dist/arg.o:
	$(CC) -c -o $@ src/arg.c $(CFLAGS) $(LDFLAGS)

dist/websocket.o:
	$(CC) -c -o $@ src/websocket.c $(CFLAGS) $(LDFLAGS)

jsonic:
	make -C lib/jsonic

clean:
	make clean -C lib/jsonic
	$(RM) dist/
	$(RM) $(EXECUTABLES)