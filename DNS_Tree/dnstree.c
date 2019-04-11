#include "dnstree.h"

int getIndex(char c)
{
        return (c == '.') ? 26 : (c - 'a');
}

int getCharFromIndex(int i)
{
        return (i == 26) ? '.' : ('a' + i);
}

trieNode *newTrieNode(void)
{
     trieNode *newNode = NULL;
     newNode = (trieNode*) malloc (sizeof(trieNode));
     newNode->isleaf = false;
     newNode->ipadd = NULL;
     for(int i=0; i<CHARS; i++)
        newNode->child[i] = NULL;

     return newNode;
}

void readData(trieNode *root, char *URL, char *ipadd)
{
        int len = strlen(URL);
        trieNode *pCrawl = root;

        for(int level = 0; level<len; level++)
        {
                int index = getIndex(URL[level]);

                if(!pCrawl->child[index])
                        pCrawl->child[index] = newTrieNode();

                pCrawl = pCrawl->child[index];
        }
        pCrawl->isleaf = true;
        pCrawl->ipadd = (char *)malloc(strlen(ipadd)+1);
        strcpy(pCrawl->ipadd, ipadd);
        }

char *lookUp(trieNode *root, char *URL)
{
          //root node of a trie
        trieNode *pCrawl = root ;
        int len = strlen(URL);

        for(int level = 0; level < len; level++)
        {
                int index = getIndex(URL[level]);
                if(!pCrawl->child[index])
                        return NULL;

                pCrawl = pCrawl->child[index];
        }

        // if we find the last node for a given ip address, print the ip address

        if(pCrawl != NULL && pCrawl->isleaf){
                 return pCrawl->ipadd;
        }
        return NULL;
}