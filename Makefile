CFLAGS += -O3 -Wall
UNAME_S = $(shell uname -s)
LDFLAGS += -lc -lm -lpcre2-8
ifeq ($(UNAME_S),Darwin)
	CFLAGS += -I/opt/local -I/usr/local/Include
	LDFLAGS +=  -L/usr/local/opt/pcre2/lib/
endif

all:
	@echo "Building core object files..."
	@$(CC) $(CFLAGS) -c src/calc.c -o bin/calc.o
	@$(CC) $(CFLAGS) -c src/cJSON.c -o bin/cJSON.o
	@echo "Building termcalc object files..."
	@$(CC) src/term_gcalc.c $(CFLAGS) -c -o bin/term_gcalc.o
	@$(CC) src/intcalc.c $(CFLAGS) -c -o bin/intcalc.o
	@$(CC) src/main.c $(CFLAGS) -c -o bin/main.o
	@echo "Linking object files..."
	@$(CC) bin/calc.o bin/cJSON.o bin/intcalc.o bin/term_gcalc.o bin/main.o $(LDFLAGS) -lpcre2-8 -o bin/termcalc
	@echo "Cleaning up object files..."
	@rm bin/*.o

shared:
	@echo "Building object files..."
	@$(CC) $(CFLAGS) -fpic -c src/calc.c -o bin/calc.o
	@$(CC) $(CFLAGS) -fpic -c src/cJSON.c -o bin/cJSON.o
	@echo "Linking shared library..."
	@$(CC) -shared bin/calc.o bin/cJSON.o $(LDFLAGS) -o bin/libcalc.so
	@echo "Cleaning up object files..."
	@rm bin/*.o

static:
	@echo "Building object files..."
	@$(CC) $(CFLAGS) -c src/calc.c -o bin/calc.o
	@$(CC) $(CFLAGS) -c src/cJSON.c -o bin/cJSON.o
	@echo "Archiving static library..."
	@ar rcs bin/libcalc.a bin/calc.o bin/cJSON.o
	@echo "Cleaning up object files..."
	@rm bin/*.o
clean:
	@rm -f bin/*.o bin/*.so bin/*.a