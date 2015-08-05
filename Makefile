CXX=gcc
CFLAGS= -Werror -Wall -g

my-wget : my-wget.o
	${CXX} ${CFLAGS} -o $@ $^

my-wget.o : my-wget.s
	${CXX} ${CFLAGS} -c -o $@ $^

my-wget.s : my-wget.c
	${CXX} ${CFLAGS} -S -o $@ $^


clean :
	rm -f my-wget my-wget.o my-wget.s
