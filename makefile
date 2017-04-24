all :
	gcc -std=c++11 -c CPU.cpp main.cpp
	gcc CPU.o main.o -o main -lsfml-graphics -lsfml-window -lsfml-system -lstdc++

