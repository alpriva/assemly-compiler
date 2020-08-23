assembler: Compiler.o LangDefs.o Main.o Parser.o Tables.o Translator.o Utilities.o
	gcc -g -Wall -ansi -pedantic Compiler.o LangDefs.o Main.o Parser.o Tables.o Translator.o Utilities.o -o assembler -lm
Compiler.o: Compiler.c Compiler.h
	gcc -c Compiler.c -o Compiler.o
LangDefs.o: LangDefs.c LangDefs.h
	gcc -c LangDefs.c -o LangDefs.o -lm
Main.o: Main.c
	gcc -c Main.c -o Main.o -lm
Parser.o: Parser.c Parser.h
	gcc -c Parser.c -o Parser.o -lm
Tables.o: Tables.c Tables.h
	gcc -c Tables.c -o Tables.o -lm
Translator.o: Translator.c Translator.h
	gcc -c Translator.c -o Translator.o -lm
Utilities.o: Utilities.c Utilities.h
	gcc -c Utilities.c -o Utilities.o -lm