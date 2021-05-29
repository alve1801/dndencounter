all: main.cpp lib.h
	g++ -o a main.cpp -lSDL2 -g
	./a
