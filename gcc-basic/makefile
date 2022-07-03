CC=g++
#CC=c++-11
CFLAGS=-Wall
OBJS=base64.o base64_main.o hello.o
all: libbase64.so hello base64_main base64_main_shared base64_main_static

hello.o:hello.cpp
# $@ is hello.o
	${CC} -Wall -g -c hello.cpp -o $@

base64_main.o: base64_main.cpp
	${CC} -Wall -g -c base64_main.cpp -o $@

base64.o: base64.cpp
	${CC} -Wall -g -c base64.cpp -o $@

hello: hello.o
	${CC} -Wall -g hello.o -o $@

base64_main: base64.o base64_main.o
	${CC} -Wall -g base64.o base64_main.o -o $@

base64_main_shared: base64_main.o libbase64.so
	${CC} -Wall -g base64_main.o -o $@ -L./ -lbase64

base64_main_static: base64_main.o libbase64.a
	${CC} -Wall -g base64_main.o libbase64.a -o $@

libbase64.so: base64.o
	${CC} -shared -o $@ base64.o -install_name @rpath/$@

libbase64.a: base64.o
	ar rvs $@ base64.o

clean:
	rm -f $(OBJS) libbase64.so libbase64.a base64_main hello base64_main_shared base64_main_static