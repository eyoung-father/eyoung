all:
	gcc -o pop3 -g -Wall -Werror -I../ -I../../../include -L../../../build/lib -lmemory -lpthread -DPOP3_MAIN=1 demo.c ../obj/_pop3.o

clean:
	rm -rf pop3
