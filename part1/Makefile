
all : bank
	
	
bank: bank.o string_parser.o
	gcc -o bank bank.o string_parser.o
	
	
bank.o: bank.c
	gcc -g -c bank.c
	
string_parser.o: string_parser.c string_parser.h
	gcc -c string_parser.c

clean:
	rm -f core *.o bank
