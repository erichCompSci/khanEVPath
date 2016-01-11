test_im7_mem: test_im7_mem.c
	g++ -o test_im7_mem test_im7_mem.c -L$(PWD) -lmyim7 -Wl,-rpath,${PWD}

all: build

build: copy test_im7_mem

copy:
	@cp ../PyScripts/libim7/libim7/_im7.so ./libmyim7.so
	@cp ../PyScripts/libim7/src/ReadIM7.h ./
	@cp ../PyScripts/libim7/src/ReadIMX.h ./

clean:
	rm ReadIM7.h ReadIMX.h _im7.so
