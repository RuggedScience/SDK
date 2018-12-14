CC=g++
CCFLAGS=
INSTALL_PATH=/usr/local

librspoe.so: controllers/pd69104.cpp poe.cpp
	$(CC) -std=c++11 -c -Wall -Werror -fPIC controllers/pd69104.cpp
	$(CC) -std=c++11 -c -Wall -Werror -fPIC poe.cpp 
	$(CC) -shared -o librspoe.so poe.o pd69104.o

clean:
	rm -f pd69104.o poe.o librspoe.so

install: lib
	mkdir -p $(INSTALL_PATH)/lib
	mkdir -p $(INSTALL_PATH)/include/librspoe
	cp librspoe.so $(INSTALL_PATH)/lib
	cp poe.h $(INSTALL_PATH)/include/librspoe/
	ldconfig

uninstall:
	rm -f $(INSTALL_PATH)/lib/librspoe.so
	rm -f $(INSTALL_PATH)/include/librspoe/poe.h