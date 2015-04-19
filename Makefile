h3c: h3c.o main.o handler.o
	cc h3c.o handler.o main.o -o h3c

h3c.o: h3c.h h3c.c
	cc -c h3c.c

main.o: main.c
	cc -c main.c

handler.o:handler.h handler.c
	cc -c handler.c

clean:
	rm *.o
.PHONY: clean
