#include <stdlib.h>
#include "Tree.h"
#include "toDot.h"


int main(){
	Tree t = createTree(3);
	FILE * fptr = fopen("tree.dot","w");
	bst_print_dot(t, fptr);
	fclose(fptr);
	system("dot -Tpng tree.dot -o tree.png");
	DFT(t);
	printf("\n");
	BFT(t);
	return 0;
}