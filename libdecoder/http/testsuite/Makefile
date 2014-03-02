all:
	gcc -o http -g -Wall -Werror -I../ -I../../../include -I../include -L../../../build/lib -ley_z -lhtml -lhttp -lengine -ljit -lmatch -ldl -lutil -lelf -lmemory -lpthread -DHTTP_MAIN=1 demo.c
	gcc -o http_html -g -Wall -Werror -I../ -I../../../include -I../include -I../../html/include -L../../../build/lib -ley_z -lhtml -lhttp -lengine -ljit -lmatch -ldl -lutil -lelf -lmemory -lpthread -DHTTP_MAIN=1 demo_http_html.c
	gcc -o http_xss -g -Wall -Werror -I../ -I../../../include -I../include -I../../html/include -L../../../build/lib -ley_z -lhtml -lhttp -lengine -ljit -lmatch -ldl -lutil -lelf -lmemory -lpthread -DHTTP_MAIN=1 demo_http_xss.c
	cp ../../../include/*.h ./include
	cp ../../../include/libtcc/*.h ./include
	cp ../include/* ./include
	cp ../../html/include/* ./include
	make -C ../detect
	cp ../detect/obj/libhttp_detect.so .
	make -C ../../html/detect
	cp ../../html/detect/obj/libhtml_detect.so .

clean:
	rm -rf http http_html http_xss http.eyc html.eyc http_xss.eyc libhttp_detect.so libhtml_detect.so log ./include/*.h

