all: test.cpp
	g++ -g -Wall -o test test.cpp -lntl

clean:
	$(RM) test

run: all
	./test
