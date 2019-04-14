#include <stdio.h>
#include <stdlib.h>
#include "ListIterator.h"
#include "Tree.h"
#include "List.h"

#define MAX_HEIGHT 3

Tree _createTree(int n, int height){
	static int counter =0;
	if(height >= MAX_HEIGHT)
		return NULL;
	Tree t = (Tree)malloc(sizeof(struct _tree));
	t->key = counter++;
	t->children = (iter)malloc(sizeof(struct _iterator));
	int i;
	iter it = t->children;
	if(n>0){
		it->t = _createTree(n, height+1);
		it->next = NULL;
	}
	for(i=1; i<n; ++i){
		it->next = (iter)malloc(sizeof(struct _iterator));
		it->next->t = _createTree(n, height+1);
		it->next->next = NULL;
		it = it->next;
	}
	return t;
}

Tree createTree(int n){
	return _createTree(n,0);
}

Boolean isEmpty(Tree t){
	if(t == NULL)
		return true;
	else
		return false;
}

int getRootVal(Tree t){
	return t->key;
}

Iterator getChildren(Tree t){
	Iterator it = (Iterator)malloc(sizeof(iter));
	*it = t->children;
	return it;
}

Tree selectNextNode(List t){
	return popFromFront(t);
}

List identifyMoreNodes(Tree t){
	return makeList(getChildren(t));
}

List addMoreNodes_BF(List l1, List l2){
	return append(l1,l2);
}

List addMoreNodes_DF(List l1, List l2){
	return append(l2,l1);
}

void BFT(Tree t){
	if(!isEmpty(t))
		printf("%d\n",t->key);
	List it = makeList(getChildren(t));
	while(!isEmptyList(it)){
		t = selectNextNode(it);
		printf("%d\n",t->key);
		it = addMoreNodes_BF(it,identifyMoreNodes(t));
	}
}

void DFT(Tree t){
	if(!isEmpty(t))
		printf("%d\n",t->key);
	List it = makeList(getChildren(t));
	while(!isEmptyList(it)){
		t = selectNextNode(it);
		printf("%d\n",t->key);
		it = addMoreNodes_DF(it,identifyMoreNodes(t));
	}	
}