#ifndef LIST_ITERATOR_H
#define LIST_ITERATOR_H

#include "Tree.h"
#include "List.h"

typedef struct _iterator * iter;

struct _iterator{
	Tree t;
	iter next;
};

typedef iter * Iterator;

Boolean hasMoreElements(Iterator);

Tree getNextElement(Iterator);

List makeList(Iterator);

#endif