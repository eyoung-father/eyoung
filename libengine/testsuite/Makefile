all:
	gcc -g -o engine main.c -I../../include -L../../build/lib/ -lpthread -lengine -lmemory -ldl -ljit -lutil -lelf -lmatch
	make -C ../../test
	cp ../../test/libtest.so .
	cp ../../include/* ./include

clean:
	rm -rf engine libtest.so test.eyc
