CC=g++
CFLAGS=-Wall
OBJS=base64.o base64_main.o hello.o
all: libbase64.so hello base64_main base64_main_shared base64_main_static

hello.o:hello.cpp
	g++ -Wall -g -c hello.cpp -o hello.o

base64_main.o: base64_main.cpp
	g++ -Wall -g -c base64_main.cpp -o base64_main.o

base64.o: base64.cpp
	g++ -Wall -g -c base64.cpp -o base64.o

hello: hello.o
	g++ -Wall -g hello.o -o hello

base64_main: base64.o base64_main.o
	g++ -Wall -g base64.o base64_main.o -o base64_main

base64_main_shared: base64_main.o libbase64.so
	g++ -Wall -g base64_main.o -o base64_main_shared -L./ -lbase64

base64_main_static: base64_main.o libbase64.a
	g++ -Wall -g base64_main.o libbase64.a -o base64_main_static

libbase64.so: base64.o
	g++ -shared -o $@ base64.o

libbase64.a: base64.o
	ar rvs libbase64.a base64.o

clean:
	rm -f $(OBJS) libbase64.so libbase64.a base64_main hello base64_main_shared base64_main_static