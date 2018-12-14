CC=g++
CCFLAGS=
INSTALL_PATH=/usr/local

librsdio.so: controllers/ite8783.cpp controllers/ite8786.cpp dio.cpp
	$(CC) -std=c++11 -c -Wall -Werror -fPIC controllers/ite8783.cpp
	$(CC) -std=c++11 -c -Wall -Werror -fPIC controllers/ite8786.cpp
	$(CC) -std=c++11 -c -Wall -Werror -fPIC dio.cpp 
	$(CC) -shared -o librsdio.so dio.o ite8786.o ite8783.o

clean:
	rm -f ite8783.o ite8786.o dio.o librsdio.so

install: lib
	mkdir -p $(INSTALL_PATH)/lib
	mkdir -p $(INSTALL_PATH)/include/librsdio
	cp librsdio.so $(INSTALL_PATH)/lib
	cp dio.h $(INSTALL_PATH)/include/librsdio/
	ldconfig

uninstall:
	rm -f $(INSTALL_PATH)/lib/librsdio.so
	rm -f $(INSTALL_PATH)/include/librsdio/dio.h