#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h> 
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>


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

//Change Directories (cd)
void cd(const char *dir){ //--does this account for PATH and HOME if typed in?
  
  if(dir == NULL){ //nothing is passed in -- understood "HOME"
    if(chdir(getenv("HOME")) == -1){ //checks for HOME directory
      printf("<%s> is not a working pathname.\n", strerror(errno));
    }
  }else{
    if(chdir(dir) == -1){ //checks for directory given
      printf("<%s> is not a working pathname.\n", strerror(errno));
    }
  }  
}

//List items within current directory (ls)
void ls(){ //will eventually need this to potentially return a char*
  
  char* cwd;
  char buf[BSIZE];
  
  cwd = getcwd( buf, BSIZE ); //current file directory
  
  DIR *dir; 
  struct dirent *dirFiles;
  dir = opendir(cwd);
  if (dir)
  {
    while ((dirFiles = readdir(dir)) != NULL) //reads files with in that directory
    {
      printf("%s\n", dirFiles->d_name);
    }
    closedir(dir);
  }
} 

//Sets the enviroment variables
void set(char* pathSet){ // -- unsure if this is setting the enviroment variables for the child process? -- also may need to create special case for PATH and multiple inputs
  
  const char* delim = '=';
  char* pathType; //either HOME or PATH
  char* value; 
  
  pathType = strtok(pathSet, delim);
  value = strtok(NULL, delim);
  
  if (setenv(pathType.c_str(), value.c_str(), 1) < 0){
    printf("<%s> cannot overwrite environment variables.\n", strerror(errno));
  }
}

//Displays jobs when user calls jobs function
void jobDisplay(){
  for (int i = 0; i < BSIZE; i++){
    if (jobs[i].alive){
      printf("[%d]\t%d\t%s\n",jobs[i].id,jobs[i].pid,jobs[i].command);
    }
  } 
  
}




//will run command in background
void runBackground(){
  
  
}