all: vhe.cpp
	g++ -O3 -Wall -o vhe vhe.cpp -lntl

clean:
	$(RM) vhe

run: all
	./vhe
