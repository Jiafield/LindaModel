all: *.cpp
	g++ -o P1 -g -Wall -std=gnu++0x -lboost_regex -lpthread tupleObjects.cpp lindaFunctions.cpp tokenizer.cpp
clean:
	rm -f *.o P1 tempFile* *~
