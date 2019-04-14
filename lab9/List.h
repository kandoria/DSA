#ifndef LIST_H
#define LIST_H

#include <stdio.h>
#include "Tree.h"

typedef struct _element * Element;

struct _element{
	Tree node;
	Element next; 
};

typedef struct _list * List;

struct _list{
	Element first;
	Element last;
};

typedef Tree Node;

List newList();

List pushBack(List, Node);

List append(List, List);

Node popFromFront(List);

Boolean isEmptyList(List);

void printList(List);

#endif
