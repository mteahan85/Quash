#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h> 
#include <sys/types.h>
#include <dirent.h>


#define BSIZE 256


typedef struct{
  bool background; 
  bool alive;
  int id;
  pid_t pid;
  char * command;
  
} Job;






int main(int argc, char **argv, char **envp)
{
  Job jobs[BSIZE];
  
  //will have to initalize things
  
  
  
  //have loop to run actual commands on
  while(1){
    
    
  }
  
  return 0; 
  
  
  
  
}

//ChangeDirectories
void cd(const char *dir){
  
  if(dir == NULL){ //nothing is passed in -- understood "HOME"
    if(chdir(getenv("HOME")) == -1){
      printf("<%s> is not a working pathname.\n", strerror(errno));
    }
  }else{
    if(chdir(dir) == -1){
      printf("<%s> is not a working pathname.\n", strerror(errno));
    }
  }  
}


void ls(){ //will eventually need this to potentially return a char*
  
    char* cwd;
    char buf[BSIZE];

    cwd = getcwd( buf, BSIZE ); //current file directory
  
  
  DIR *dir; 
  struct dirent *dirFiles;
  dir = opendir(cwd);
  if (dir)
  {
    while ((dirFiles = readdir(dir)) != NULL)
    {
      printf("%s\n", dirFiles->d_name);
    }

    closedir(dir);
  }
}  
  

void jobs(){
    for (int i = 0; i < BSIZE; i++){
      if (jobs[i].alive){
	printf("[%d]\t%d\t%s\n",jobs[i].id,jobs[i].pid,jobs[i].command);
      }
    } 

}

//will run command in background
void runBackground(){
  
  
}