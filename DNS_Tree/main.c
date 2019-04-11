#include "dnstree.h"

int main(){
         int key,N;
         trieNode *dnstree = newTrieNode();

         printf("Enter the Key value to perform required action or press -1 to exit: ");
         scanf("%d",&key);

        while(key != -1){
        if(!(key == 1||key == 2||key == -1)){
                printf("Enter either 1 or 2 or -1\n");
         }

        else if(key==1){
                scanf("%d", &N);
                char URL[N];
                char ip[N];
                for(int i = 0; i < N;i++){
                      scanf("%s",&URL[i]);
                      scanf("%s",&ip[i]);
                      readData(dnstree, URL,ip);
                }
         }
       else if(key == 2){
                char url[MAX];
                printf("Enter the key to look up: ");
                scanf("%s",url);
                char *res_ip = lookUp(dnstree, url);
                printf("The resultant ip is : %s\n",res_ip);
}
                printf("Enter the Key value to perform required action or press -1 to exit: ");
                scanf("%d",&key);
}
return 0;
}