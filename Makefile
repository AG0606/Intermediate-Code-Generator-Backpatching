CC = gcc
CFLAGS = -Wall -Wextra -Isrc -g
LEX = flex
YACC = bison -d

SRCS = src/y.tab.c src/lex.yy.c src/quad.c src/symbol_table.c src/loop_stack.c src/cfg.c src/optimizer.c src/error_reporter.c src/main.c
OBJS = $(SRCS:.c=.o)
TARGET = icg

all: $(TARGET)

$(TARGET): src/y.tab.c src/lex.yy.c $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

src/y.tab.c src/y.tab.h: src/parser.y
	$(YACC) -o src/y.tab.c src/parser.y

src/lex.yy.c: src/lexer.l src/y.tab.h
	$(LEX) -o src/lex.yy.c src/lexer.l

test: $(TARGET)
	python3 run_tests.py

clean:
	rm -f $(TARGET) src/lex.yy.c src/y.tab.c src/y.tab.h src/*.o *.dot *.html quads.json
