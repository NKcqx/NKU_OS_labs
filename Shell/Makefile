CC=gcc
EXE=test1

all:${EXE}

test1:test1.o
	${CC} -o $@ $<

test1.o:test1.c
	${CC} -c $<

run:${EXE}
	./test1

clean:
	rm -fr ${EXE} *.o