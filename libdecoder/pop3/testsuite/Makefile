all:
	gcc -o pop3 -g -Wall -Werror -I../ -I../../../include -I../include -L../../../build/lib -lpop3 -lengine -ljit -lmatch -ldl -lutil -lelf -lmemory -lpthread -DPOP3_MAIN=1 demo.c
	cp ../../../include/*.h ./include
	cp ../../../include/libtcc/*.h ./include
	cp ../include/* ./include
	make -C ../detect
	cp ../detect/obj/libpop3_detect.so .

clean:
	rm -rf pop3 pop3.eyc libpop3_detect.so log ./include/*.h

