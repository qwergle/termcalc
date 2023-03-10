CC = gcc

all:
	@echo "Building shared library object files..."
	@$(CC) -fPIC -O3 -c calc.c cJSON.c -lpcre2-8
	@echo "Linking shared library..."
	@$(CC) -shared -O3 calc.o cJSON.o -lpcre2-8 -o libgcalc.so
	@echo "Cleaning up object files..."
	@rm calc.o
	@rm cJSON.o
	@echo "Building term_gcalc..."
	@$(CC) term_gcalc.c -O3 -L./ -lgcalc -o term_gcalc
	@echo "Building interactive_calc.c..."
	@$(CC) intcalc.c -O3 -L./ -lgcalc -o interactive_calc

clean:
	@rm libgcalc.so
	@rm term_gcalc
	@rm interactive_calc