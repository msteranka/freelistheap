# clang++ -std=c++11 -pthread -g -O2 -DNDEBUG -Wall -shared libodin.cpp -o libodin.so

all:
	clang++ -pthread -std=c++11 -o main main.cpp

