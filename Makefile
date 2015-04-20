# build an executable named myprog from myprog.c

all: test.cpp
	g++ -g -Wall -o test test.cpp -lntl

clean:
	$(RM) test