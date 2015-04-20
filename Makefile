all: test.cpp
	g++ -g -Wall -o test test.cpp -lntl -lgmp

clean:
	$(RM) test

run: all
	./test
