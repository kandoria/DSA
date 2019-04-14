#include "List.h"
#include <stdlib.h>

List newList(){
	List s = (List)malloc(sizeof(struct _list));
	s->first = NULL;
	s->last = NULL;
	return s;
}

List pushBack(List s, Tree node){
	Element n = (Element)malloc(sizeof(struct _element));
	n->node = node;
	n->next = NULL;
	if(s->first == NULL){
		s->first = n;
		s->last = n;
		return s;
	}
	else {
		s->last->next = n;
		s->last = n;
	}
	return s;
}

List append(List a, List b){
	if(a == NULL || a->first == NULL)
		return b;
	if(b == NULL || b->first == NULL)
		return a;
	a->last->next = b->first;
	a->last = b->last;
	return a;
}

Tree popFromFront(List s){
	if (s->first == NULL){
		return NULL;
	}
	Element n = s->first;
	Tree node = n->node;
	s->first = n->next;
	free(n);
	return node;
}

Boolean isEmptyList(List s){
	if(s == NULL || s->first == NULL)
		return true;
	else
		return false;
}

void printList(List s){
	if(isEmptyList(s)){
		printf("List is empty!\n");
		return;
	}
	Element a = s->first;
	while(a!=NULL){
		printf("%d\t",a->node->key);
		a = a->next;
	}
	printf("\n");
}