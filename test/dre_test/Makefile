#!/bin/sh
#
# HMCSIM TEST APPLICATIONS 
#
# TEST: DRETEST
# ---------------------------------------

include ../Makefile.inc

SRC=./src

OBJS=$(SRC)/genrand.o $(SRC)/execute_test.o $(SRC)/shiftamt.o

all: dretest
$(SRC)/execute_test.o: $(SRC)/execute_test.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/genrand.o: $(SRC)/genrand.c
	$(CC) $(CFLAGS) -c -o $@ $<
$(SRC)/shiftamt.o: $(SRC)/shiftamt.c
	$(CC) $(CFLAGS) -c -o $@ $<
dretest: $(SRC)/genrand.o $(SRC)/execute_test.o $(SRC)/shiftamt.o
	$(CC) $(CFLAGS) -c -o $(SRC)/dretest_main.o $(SRC)/dretest_main.c
	$(CC) $(CFLAGS) -o $@ $(SRC)/dretest_main.o $(OBJS) $(LDFLAGS) $(LDLIBS)
clean: 
	rm -Rf ./dretest ./src/*.o
