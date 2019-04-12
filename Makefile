CXX := g++
OPTIONS := -std=c++17 -Wall -pedantic

all: find

find: find.o filter.o
	$(CXX) $(OPTIONS) find.o filter.o -o find

find.o:
	$(CXX) $(OPTIONS) -c find.cpp

filter.o:
	$(CXX) $(OPTIONS) -c filter.cpp

clean:
	rm -rf *.o find
