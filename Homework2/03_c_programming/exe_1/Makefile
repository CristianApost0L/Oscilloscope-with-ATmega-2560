CC=gcc
CCOPTS=--std=gnu99 -Wall -D_LIST_DEBUG_ -g 
AR=ar	#libreria statica

OBJS=linked_list.o 

HEADERS=linked_list.h

LIBS=liblinkedlist.a

BINS=linked_list_test

#disastros_test

.phony: clean all


all:	$(BINS) $(LIBS)

%.o:	%.c $(HEADERS)
	echo "sto a compilando evero un puntoci in un puntoo"
	$(CC) $(CCOPTS) -c -o $@  $<

# $@ nome di testa, $< nome in coda

liblinkedlist.a:	$(OBJS) 
	echo "genero la libreria"
	$(AR) -rcs $@ $^

# $@ = liblinkedlist.a, $^ = linked_list.o

linked_list_test:	linked_list_test.c $(LIBS)
	echo "comopilo il main"
	$(CC) $(CCOPTS) -o $@ $^

clean:
	echo "lavo"
	rm -rf *.o *~ $(LIBS) $(BINS)

# clean making
