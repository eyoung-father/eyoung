all:
	gcc -o pop3 -g -Wall -Werror -I../ -I../../../include -I../include -L../../../build/lib -lpop3 -lpop3_detect -lmemory -lpthread -DPOP3_MAIN=1 demo.c

clean:
	rm -rf pop3
