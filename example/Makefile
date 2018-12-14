CC=g++
CCFLAGS=

all: exampledio examplepoe

exampledio: exampledio.cpp
	$(CC) -std=c++11 exampledio.cpp -lrsdio -o exampledio

examplepoe: examplepoe.cpp
	$(CC) -std=c++11 examplepoe.cpp -lrspoe -o examplepoe

clean:
	rm -rf exampledio examplepoe