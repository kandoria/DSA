#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int count;

void DFTFile(char* path){
	DIR *dir;
	struct dirent *sd;
	struct stat fileStat;
	dir = opendir(path);
	if(dir == NULL){
		//printf("Returning from %s because not a directory\n",path);
		//fprintf(stderr, "stat() error on %s: %s\n", path,strerror(errno));
		return;
	}
	printf("\n\n\t\tContents of: %s\n\n",path);
	char p[1400];
	while((sd = readdir(dir)) != NULL){
		if(strcmp(sd->d_name,".") && strcmp(sd->d_name,"..")){
			printf("\t%d. \t%s\n",count++,sd->d_name);
			int f = open(sd->d_name,O_RDONLY);
			fstat(f,&fileStat);
//			if(!(S_ISREG(fileStat.st_mode))){
			strcpy(p,path);
			if(strcmp(p,"/"))
				strcat(p,"/");
			strcat(p,sd->d_name);
			DFTFile(p);
//			}
		}
	}
	printf("\n\n\t\tEnd of contents of: %s\n\n",path);
	closedir(dir);
}

int main(){
	count = 1;
	DFTFile("/");
	return 0;
}