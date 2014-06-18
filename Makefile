odir=obj
crparse.o:
	gcc -ggdb -c creole_parse.c -o $(odir)/crparse.o

full: crparse.o
	gcc -ggdb -c entry.c -o $(odir)/entry.o
	gcc $(odir)/crparse.o $(odir)/entry.o -o crp
