
all: h3c

md5.o: ./md5/md5.c
	$(CC) $(CFLAGS) -c $< -o $@

h3c: h3c.o main.o handler.o echo.o md5.o

clean:
	rm *.o
.PHONY: clean
