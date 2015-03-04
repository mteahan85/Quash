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
#include <termios.h>



#define BSIZE 256

pid_t shellPID;
pid_t shellPGID;
struct termios shellTmodes;
int shellTerminal;
int shellIsInteractive;

static int qargc=0;
static char qargv[10]; //change if too small
static char cmdBuf[BSIZE];
static int bufChar=0;

static char in;
//hosting data/name stuff
char hostname[BSIZE];
char *directory;
char *username;


typedef struct{
  bool background; 
  bool alive;
  int id;
  pid_t pid;
  char * command;
  
} Job;






int main(int argc, char **argv, char **envp)
{
  //let's just designate things to individual files, so they look contained and pretty
  
  printf("##############################################\n");
  printf("#                   Quash                    #\n");
  printf("# Written by: Nicole Maneth and Megan Teahan #\n");
  printf("##############################################\n");
  
  //initialize shell
  initializeShell();
  
  while(1){
   in = getchar();
   if(in=='\n'){	//no input    
    begLineDisplay();
   }
   else{	//they typed something; deal with it
     readCommand();
     performCommand();
     begLineDisplay();
   }
  } 
   
}

void begLineDisplay(){
  printf("%s:%s %s$ ", hostname, directory, username);
}

void initializeShell(){
  shellPID = getpid();
  shellTerminal = STDIN_FILENO;
  shellIsInteractive = isatty(shellTerminal);

  if (shellIsInteractive) {
    //loop until shell is in the foreground
    while (tcgetpgrp(shellTerminal) != (shellPGID = getpgrp())){
	    kill(-shellPGID, SIGTTIN);
    }
    // Ignore interactive and job-control signals.  
    signal (SIGINT, SIG_IGN);
    signal (SIGQUIT, SIG_IGN);
    signal (SIGTSTP, SIG_IGN);
    signal (SIGTTIN, SIG_IGN);
    signal (SIGTTOU, SIG_IGN);
    signal (SIGCHLD, SIG_IGN);

    //put shell in own process group
    shellPGID = getpgrp();
    if (setpgid(shellPID, shellPID)<0) {
      printf("Error, the shell is not process group leader");
      exit(1);
    }
    
    //grab control of the terminal
    tcsetpgrp(shellTerminal, shellPGID);
    //save default terminal attributes for the shell
    tcgetattr(shellTerminal, &shellTmodes); 
    
    //save directory
    directory = (char*) malloc(sizeof(char)*1024);
  }
  else{
    printf("Failed to make shell interactive; quitting now \n");
    exit(1);
  }
  begLineDisplay();
  fflush(stdout);
}

void readCommand(){ //parses input
  //initialize
  while(qargc!=0){
    qargv[qargc] = NULL;
    qargc--;
  }
  bufChar = 0;
  
  //store input in cmdBuf array
  char *bufPtr;
  while(in != '\n'){
    cmdBuf[bufChar]= in;
    bufChar++;
    in= getchar();
  }
  
  //put mark at end of input
  cmdBuf[bufChar] = 0x00;
  //break it up by spaces
  bufPtr = strtok(cmdBuf, " "); //get first token
  while(bufPtr !=NULL){	//walk token through other tokens
   qargv[qargc] = bufPtr;
   bufPtr = strtok(NULL, " ");
   qargc++;
  }
  
}

void performCommand(){
  if((strcmp("exit", qargv[0])==0) || (strcmp("quit", qargv[0])==0)){
   exit(0); 
  }
  if(strcmp("cd", qargv[0])==0){
   cd(); 
  }
  if(strcmp("ls", qargv[0])==0){
   ls();
  }
  if(strcmp("jobs", qargv[0])==0){
   jobDisplay();    
  }
  if(strcmp("set", qargv[0])==0){
   set(); 
  }
  
  doJob(qargv, "STANDARD");
  
}

void doJob(char *command[], char *file){
  
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