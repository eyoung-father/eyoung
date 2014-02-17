all:
	gcc -o http -g -Wall -Werror -I../ -I../../../include -I../include -L../../../build/lib -lhttp -lengine -ljit -lmatch -ldl -lutil -lelf -lmemory -lpthread -DHTTP_MAIN=1 demo.c
	cp ../../../include/*.h ./include
	cp ../../../include/libtcc/*.h ./include
	cp ../include/* ./include
	make -C ../detect
	cp ../detect/obj/libhttp_detect.so .

clean:
	rm -rf http http.eyc libhttp_detect.so log ./include/*.h

