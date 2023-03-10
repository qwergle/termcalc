CC = gcc

all:
	@echo "Building libccalc object files..."
	@$(CC) -fPIC -O3 -c src/calc.c src/cJSON.c
	@echo "Linking libccalc..."
	@$(CC) -shared calc.o cJSON.o -lpcre2-8 -o libccalc.so
	@echo "Cleaning up object files..."
	@rm calc.o
	@rm cJSON.o
	@echo "Building termcalc object files..."
	@$(CC) src/term_gcalc.c -O3 -c
	@$(CC) src/intcalc.c -O3 -c
	@$(CC) src/main.c -O3 -c
	@echo "Linking termcalc object files..."
	@$(CC) intcalc.o term_gcalc.o main.o -L./ -lccalc -o termcalc
	@echo "Cleaning up termcalc object files..."
	@rm intcalc.o term_gcalc.o main.o

clean:
	@rm libccalc.so
	@rm termcalc