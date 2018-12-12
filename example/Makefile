CC=g++
CCFLAGS=

all: dio poe

dio:
	$(CC) -std=c++11 exampledio.cpp -lrsdio -o exampledio

poe:
	$(CC) -std=c++11 examplepoe.cpp -lrspoe -o examplepoe

clean:
	rm -rf exampledio examplepoe