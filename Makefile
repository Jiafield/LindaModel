all: *.cpp
	g++ -o tuple -g -Wall -std=c++0x *.cpp
clean:
	rm -f *.o tuple
