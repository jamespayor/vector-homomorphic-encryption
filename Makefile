
all: run

compile: vhe.cpp
	g++ -O3 -Wall -o vhe vhe.cpp -lntl -lgmp

clean:
	$(RM) vhe

run: compile
	./vhe
