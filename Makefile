CC = gcc
CFLAGS = -Wall -Wextra -Isrc -g
LEX = flex
YACC = bison -d

SRCS = src/y.tab.c src/lex.yy.c src/quad.c src/symbol_table.c src/loop_stack.c src/cfg.c src/optimizer.c src/error_reporter.c src/main.c
OBJS = $(SRCS:.c=.o)
TARGET = icg

.PHONY: all test clean help

all: $(TARGET)

$(TARGET): src/y.tab.c src/lex.yy.c $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

src/y.tab.c src/y.tab.h: src/parser.y
	$(YACC) -o src/y.tab.c src/parser.y

src/lex.yy.c: src/lexer.l src/y.tab.h
	$(LEX) -o src/lex.yy.c src/lexer.l

test: $(TARGET)
	python3 tests/run_tests.py

clean:
	rm -f $(TARGET) $(TARGET).exe src/lex.yy.c src/y.tab.c src/y.tab.h src/*.o *.dot *.html quads.json manuscript/*.aux manuscript/*.log manuscript/*.out

help:
	@echo "Available Makefile targets:"
	@echo "  make        - Build the Intermediate Code Generator (icg binary)"
	@echo "  make test   - Run the full 14-benchmark automated verification suite"
	@echo "  make clean  - Remove all compiled binaries and intermediate build files"
	@echo "  make help   - Display this help message"
