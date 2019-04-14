#ifndef TREE_H
#define TREE_H

typedef enum Boolean {false, true} Boolean;

typedef struct _iterator * iter;

typedef iter * Iterator;

struct _tree{
	int key;
	iter children;
};

typedef struct _tree * Tree;

Tree createTree(int);

Boolean isEmpty(Tree);

int getRootVal(Tree);

Iterator getChildren(Tree);

void DFT(Tree);

void BFT(Tree);

#endif