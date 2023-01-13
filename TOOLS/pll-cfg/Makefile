OBJ	:= T41_PLL T40_PLL


all: $(OBJ)

T41_PLL: T41_PLL.o
	gcc -o $@ $^

T40_PLL: T40_PLL.o
	gcc -o $@ $^

clean:
	rm -f *.o $(OBJ)

