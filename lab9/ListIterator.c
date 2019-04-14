#include <stdlib.h>
#include "ListIterator.h"
#include "List.h"

Boolean hasMoreElements(Iterator it){
	if(it != NULL && *it != NULL && (*it)->t != NULL)
		return true;
	else
		return false;
}

Tree getNextElement(Iterator it){
	if(!hasMoreElements(it))
		return NULL;
	Tree t = (*it)->t;
	*it = (*it)->next;
	return t;
}

List makeList(Iterator it){
	List s = newList();
	while(hasMoreElements(it)){
		s = pushBack(s,getNextElement(it));
	}
	return s;
}