TARGET = parser
OBJECT = parser.tab.c parser.tab.o lex.yy.c alloc.o functions.o semanticAnalysis.o symbolTable.o codeGenerate.o
OUTPUT = parser.output parser.tab.h
CC = gcc -g -Wall
LEX = flex
YACC = bison -v
YACCFLAG = -d
LIBS = -lfl 

parser: parser.tab.o alloc.o functions.o symbolTable.o semanticAnalysis.o codeGenerate.o 
	$(CC) -o $(TARGET) parser.tab.o alloc.o functions.o symbolTable.o semanticAnalysis.o codeGenerate.o $(LIBS)

parser.tab.o: parser.tab.c lex.yy.c alloc.o functions.c symbolTable.o semanticAnalysis.o codeGenerate.o
	$(CC) -c parser.tab.c
    
semanticAnalysis.o: semanticAnalysis.c symbolTable.o
	$(CC) -c semanticAnalysis.c

symbolTable.o: symbolTable.c
	$(CC) -c symbolTable.c

lex.yy.c: lexer3.l
	$(LEX) lexer3.l

parser.tab.c: parser.y 
	$(YACC) $(YACCFLAG) parser.y

alloc.o: alloc.c
	$(CC) -c alloc.c
	
functions.o: functions.c
	$(CC) -c functions.c

codeGenerate.o: codeGenerate.c
	$(CC) -c codeGenerate.c
clean:
	rm -f $(TARGET) $(OBJECT) $(OUTPUT)

