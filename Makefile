odir=obj
crparse.o:
	gcc -ggdb -std=c99 -c creole_parse.c -o $(odir)/crparse.o

full: crparse.o
	gcc -ggdb -std=c99 -c entry.c -o $(odir)/entry.o
	gcc -std=c99 $(odir)/crparse.o $(odir)/entry.o -o crp
