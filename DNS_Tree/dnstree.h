#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>

#define CHARS 27

#define MAX 100

typedef struct node
{
         bool isleaf;
         char *ipadd;
         struct node *child[CHARS];
}trieNode;

int getIndex(char c);
int getCharFromIndex(int i);
trieNode *newTrieNode(void );
void readData(trieNode *root, char *URL, char *ipAdd);
char *lookUp(trieNode *root, char *URL);

