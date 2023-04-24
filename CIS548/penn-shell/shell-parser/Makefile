demo: demo.c parser.c
	clang -g -o demo -Wall -Werror demo.c parser.c

release: parser.c
	clang -o parser.o -Wall -Werror -c -O3 parser.c

clean: 
	rm demo; rm *.o
