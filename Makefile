CFLAGS += -O3 -Wall
UNAME_S = $(shell uname -s)
LDFLAGS += -lc -lm
ifeq ($(UNAME_S),Darwin)
	CFLAGS += -I/opt/local -I/usr/local/Include
	LDFLAGS +=  -L/usr/local/opt/pcre2/lib/
endif

all:
	@echo "Building core object files..."
	@$(CC) $(CFLAGS) -c src/calc.c src/cJSON.c
	@echo "Building termcalc object files..."
	@$(CC) src/term_gcalc.c src/intcalc.c src/main.c $(CFLAGS) -c
	@echo "Linking object files..."
	@$(CC) calc.o cJSON.o intcalc.o term_gcalc.o main.o $(LDFLAGS) -lpcre2-8 -o termcalc
	@echo "Cleaning up object files..."
	@rm calc.o cJSON.o intcalc.o term_gcalc.o main.o

clean:
	@rm termcalc