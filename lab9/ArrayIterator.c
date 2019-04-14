#include <stdlib.h>
#include "ArrayIterator.h"

Boolean hasMoreElements(Iterator it){
	if(it!=NULL && *it!=NULL && (*it)->arr[(*it)->index] != NULL)
		return true;
	else
		return false;
}

Tree getNextElement(Iterator it){
	if(!hasMoreElements(it))
		return NULL;
	Tree t = (*it)->arr[(*it)->index];
	++((*it)->index);
	return t;
}