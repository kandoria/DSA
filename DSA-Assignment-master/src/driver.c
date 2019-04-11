#include <../include/linkedlist.h>

int main(){

	initializeStack();

	int uin = NIL;
	int listcount=0;
	int listno;
	int LISTSIZE = SIZE;
	node_object elem;
	int status;
	int thecount;
	LinkedList** listarray = (LinkedList**)malloc(sizeof(LinkedList*)*LISTSIZE);
	int* trackarr = (int*)malloc(sizeof(int)*LISTSIZE);

	printf("Welcome to the program! Written by Abhinav Ramachandran, 2017A7PS1176P on 2.1.19.");
	while(uin!=0){

		printf("\nSelect an option:\n1. Create a new list\n2. Insert a new element in a given list in sorted order\n3. Delete an element from a given list\n4. Count total elements excluding free list\n5. Count total elements of a list\n6. Display all lists\n7. Display free list\n8. Perform defragmentation\nPress 0 to exit\n");
		scanf("%d",&uin);
		switch(uin){
		case 1:
			listarray[listcount++] = createList();
			if(listcount==LISTSIZE){				
				//need to double listarray and trackarr
				LISTSIZE*=2;

				//doubling trackarr
				int* newtrackarr = (int*)malloc(sizeof(int)*LISTSIZE);
				free(trackarr);
				trackarr=newtrackarr;

				LinkedList** newlistarray = (LinkedList**)malloc(sizeof(LinkedList*)*LISTSIZE);
				//copying to the new location
				for(int i=0;i<listcount;i++){
					newlistarray[i]=listarray[i];
				}
				free(listarray);
				listarray = newlistarray;

			}
			printf("The sequence number of the newly created list is: %d.\n",listcount);
			printf("Enter key value to be inserted in the newly created list:");
			scanf("%d",&elem);
			status = insertSorted(listarray[listcount-1],elem);
			if(status==NIL){
				printf("FAILURE: MEMORY NOT AVAILABLE\n");
				printf("Deleting List.");
				listcount--;
			}
			else{
				printf("SUCCESS\n");
			}

			break;
		case 2:
			printf("List you want to insert in(ONE INDEXED):");
			scanf("%d",&listno);
			if(listno>listcount||listno<1){
				printf("Invalid list number! Returning to main menu.\n");
				break;
			}
			printf("Enter the key value:");
			scanf("%d",&elem);
			status = insertSorted(listarray[listno-1],elem);
			if(status==NIL){
				printf("FAILURE: MEMORY NOT AVAILABLE\n");
			}
			else{
				printf("SUCCESS\n");
			}
			break;
		case 3:
			printf("List you want to delete from(ONE INDEXED):");
			scanf("%d",&listno);
			if(listno>listcount||listno<1){
				printf("Invalid list number! Returning to main menu.\n");
				break;
			}
			printf("\nPlease enter the element:");
			scanf("%d",&elem);
			status = deleteElement(listarray[listno-1],elem);
			if(status==NIL){
				printf("FAILURE: ELEMENT NOT THERE / LIST EMPTY\n");
			}
			else{
				printf("SUCCESS\n");
			}
			break;
		case 4:
			thecount=0;
			for(int i=0;i<listcount;i++){
				thecount += count(listarray[i]);
			}
			printf("Total number of nodes in all the lists is %d.\n",thecount);
			break;
		case 5:	
			printf("Enter the list number:(ONE INDEXED):");
			scanf("%d",&listno);
			if(listno>listcount||listno<1){
				printf("Invalid list number! Returning to main menu.\n");
				break;
			}
			printf("Total number of nodes in list %d is %d.\n",listno,count(listarray[listno-1]));
			break;
		case 6:
			for(int i=0;i<listcount;i++){
				if(count(listarray[i])==0){
					printf("List number %d is empty!\n",i+1);
				}
				else{
					printf("Elements of list %d (of size %d) are:\n",i+1,count(listarray[i]));
					printList(listarray[i]);
				}
			}
			break;
		case 7:
			displayFree();
			break;
		case 8:
			//trackarr is used to keep track of the movements of the heads of the lists, if swapped
			//it is passed by reference to enable changing in the function
			for(int i=0;i<listcount;i++){
				trackarr[i]=listarray[i]->head;
			}
			status = defrag(trackarr,listcount);
			if(status==NIL){
				printf("FAILURE (no free nodes to defragment)\n");
			}
			else{
				printf("SUCCESS\n");
				for(int i=0;i<listcount;i++){
					listarray[i]->head = trackarr[i];
				}
			}
			break;
		case 9:
			//USED FOR DEBUGGING PURPOSES: prints the entire memory
			printf("FreePointer is %d.\n",freePointer);
			printf("LISTSIZE is %d\n",LISTSIZE);
			for(int i=0;i<SIZE;i++){
				int j=i*3;
				printf("%d :%d %d %d\n",j, memory[j],memory[j+1],memory[j+2]);
			}
			break;
		case 10:
			//USED FOR DEBUGGING PURPOSES: tests the swap function
			printf("SWAPTEST\n");
			int a,b;
			scanf("%d",&a);
			scanf("%d",&b);
			swap(a,b,NULL,0);
			break;
		}
	}
	return 0;
}
