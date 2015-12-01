#!/bin/sh
#
# HMCSIM TEST APPLICATIONS 
#
# TEST: IMGDIFF
# ---------------------------------------

include ../Makefile.inc

SRC=./src

OBJS=$(SRC)/genimg.o $(SRC)/execute_test.o $(SRC)/shiftamt.o

all: imgdiff
$(SRC)/execute_test.o: $(SRC)/execute_test.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/genimg.o: $(SRC)/genimg.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/shiftamt.o: $(SRC)/shiftamt.c
	$(CC) $(CFLAGS) -c -o $@ $<
imgdiff: $(SRC)/genimg.o $(SRC)/execute_test.o $(SRC)/shiftamt.o
	$(CC) $(CFLAGS) -c -o $(SRC)/imgdiff_main.o $(SRC)/imgdiff_main.c
	$(CC) $(CFLAGS) -o $@ $(SRC)/imgdiff_main.o $(OBJS) $(LDFLAGS) $(LDLIBS)
clean: 
	rm -Rf ./imgdiff ./src/*.o
