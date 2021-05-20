CC=gcc
OPTS=-g -std=c99 -Werror -O3 -I../src/

all: predictor_tests.o predictor.o
	$(CC) $(OPTS) -o predictor_tests predictor_tests.o predictor.o -lm

predictor_tests.o: predictor_tests.c ../src/predictor.h
	$(CC) $(OPTS) -c predictor_tests.c

clean:
	rm -f *.o predictor_tests;
