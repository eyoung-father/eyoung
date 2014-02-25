all:
	gcc -o html -g -Wall -Werror -I../ -I../../../include -I../include -L../../../build/lib -lhtml -lengine -ljit -lmatch -ldl -lutil -lelf -lmemory -lpthread -DHTML_MAIN=1 html.c
	gcc -o decode -g -Wall -Werror -I../ -I../../../include -I../include -L../../../build/lib -lhtml -lengine -ljit -lmatch -ldl -lutil -lelf -lmemory -lpthread -DHTML_MAIN=1 decode.c
	gcc -o detect -g -Wall -Werror -I../ -I../../../include -I../include -L../../../build/lib -lhtml -lengine -ljit -lmatch -ldl -lutil -lelf -lmemory -lpthread -DHTML_MAIN=1 detect.c
	cp ../../../include/*.h ./include
	cp ../../../include/libtcc/*.h ./include
	cp ../include/* ./include
	make -C ../detect
	cp ../detect/obj/libhtml_detect.so .

clean:
	rm -rf html decode detect html.eyc libhtml_detect.so log ./include/*.h

