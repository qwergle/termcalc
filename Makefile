CFLAGS += -O3 -Wall
UNAME_S = $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	CFLAGS += -I/opt/local -I/usr/local/Include
	LFLAGS =  -L/usr/local/opt/pcre2/lib/
endif

all:
	@# Include paths (for PCRE2)
	@# Compilation
	@echo "Building libccalc object files..."
	@$(CC) -fPIC $(CFLAGS) -c src/calc.c src/cJSON.c
	@echo "Linking libccalc..."
	@$(CC) -shared calc.o cJSON.o $(LFLAGS) -lpcre2-8 -o libccalc.so
	@echo "Cleaning up object files..."
	@rm calc.o
	@rm cJSON.o
	@echo "Building termcalc object files..."
	@$(CC) src/term_gcalc.c $(CFLAGS) -c
	@$(CC) src/intcalc.c $(CFLAGS) -c
	@$(CC) src/main.c $(CFLAGS) -c
	@echo "Linking termcalc object files..."
	@$(CC) intcalc.o term_gcalc.o main.o -L./ -lccalc -o termcalc
	@echo "Cleaning up termcalc object files..."
	@rm intcalc.o term_gcalc.o main.o

clean:
	@rm *.o
	@rm libccalc.so
	@rm termcalc