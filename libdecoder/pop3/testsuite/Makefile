all:
	gcc -o pop3 -g -Wall -Werror -I../ -I../../../include -I../include -L../../../build/lib -lpop3 -lengine -ljit -lmatch -ldl -lutil -lelf -lpop3_detect -lmemory -lpthread -DPOP3_MAIN=1 demo.c
	cp ../../../include/* ./include
	cp ../include/* ./include

clean:
	rm -rf pop3
