Makefile:
all:
	g++ main.cpp -lpthread -std=c++11
clean:
	rm a.out
run:
	sudo ./a.out 1 0
	sudo ./a.out 2 0
	sudo ./a.out 3 0

run2:
	sudo ./a.out 1 1
	sudo ./a.out 2 1
	sudo ./a.out 3 1
