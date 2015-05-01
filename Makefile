all: vhe.cpp
	g++ -g -Wall -o vhe vhe.cpp -lntl

clean:
	$(RM) vhe

run: all
	./vhe
