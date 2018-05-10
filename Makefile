Makefile:
all:
	g++ main.cpp -lpthread -std=c++11
clean:
	rm a.out
run:
	sudo ./a.out 1
	sudo ./a.out 2
	sudo ./a.out 3
