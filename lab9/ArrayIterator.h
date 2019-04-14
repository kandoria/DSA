#ifndef ARRAY_ITERATOR_H
#define ARRAY_ITERATOR_H

#include "Tree.h"

typedef struct _iterator * iter;

typedef iter * Iterator;

struct _iterator{
	int index;
	Tree *arr;
};

Boolean hasMoreElements(Iterator);

Tree getNextElement(Iterator);

#endif