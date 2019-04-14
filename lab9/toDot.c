#include <stdio.h>
#include "Tree.h"
#include "ListIterator.h"

void bst_print_dot_null(int key, int nullcount, FILE* stream){
    fprintf(stream, "    null%d [shape=point];\n", nullcount);
    fprintf(stream, "    %d -> null%d;\n", key, nullcount);
}

void bst_print_dot_aux(Tree node, FILE* stream){
    static int nullcount = 0;
    Iterator it = getChildren(node);
    while(hasMoreElements(it)){
        Tree t = getNextElement(it);
        fprintf(stream, "    %d -> %d;\n", getRootVal(node), getRootVal(t));
        bst_print_dot_aux(t, stream);
    }
}

void bst_print_dot(Tree tree, FILE* stream){
    fprintf(stream, "digraph BST {\n");
    fprintf(stream, "    node [fontname=\"Arial\"];\n");
    if (!tree)
        fprintf(stream, "\n");
    else if (tree->children == NULL || tree->children->t == NULL)
        fprintf(stream, "    %d;\n", getRootVal(tree));
    else
        bst_print_dot_aux(tree, stream);
    fprintf(stream, "}\n");
}