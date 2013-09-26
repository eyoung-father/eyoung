all:
	gcc -o elf -g -I../include elf_test.c -lelf
	gcc -o libtest.so -shared -g -fPIC export_test.c -I../include

clean:
	rm -rf libtest.so elf
