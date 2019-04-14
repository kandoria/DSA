fileDFT:			fileTraverse.c
	gcc fileTraverse.c -o fileDFT

test:				driver.o Tree.o List.o ListIterator.o toDot.o
	gcc driver.o Tree.o List.o ListIterator.o toDot.o -o test

driver.o:			driver.c Tree.h toDot.h
	gcc -c driver.c

Tree.o:				Tree.c Tree.h ListIterator.h toDot.h
	gcc -c Tree.c

List.o:				List.c List.h Tree.h
	gcc -c List.c

ListIterator.o:		ListIterator.c ListIterator.h Tree.h
	gcc -c ListIterator.c

ArrayIterator.o:	ArrayIterator.c ArrayIterator.h Tree.h
	gcc -c ArrayIterator.c

toDot.o:			toDot.c toDot.h Tree.h ListIterator.h
	gcc -c toDot.c