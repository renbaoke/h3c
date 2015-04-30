

h3c: h3c.o main.o handler.o echo.o
	cc -o h3c $^

clean:
	rm *.o
.PHONY: clean
