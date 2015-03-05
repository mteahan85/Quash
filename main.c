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


typedef struct Job{
  bool background; //if the job is running in the back ground
  bool alive; //if the job is alive or not
  int id;
  pid_t pid;
  char * command; //an actual command cd, ls, set, etc...
  char* fileName; //this will depend if it is an executable
  char** arguments; //an array of arguments that go with that command
  

};

static struct Job jobs[100]; //should it be Job job[];
static int *jobCount;



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


//will also need to perform a special case of parsing for '|', '<', '>' case
//and for '&'
//should probably be storing this information in
//a struct job and then adding this job to the global struct Job jobs[]
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


//may want this to take in a job,
//and then read use the information from this job to run
//maybe method looks like this: performCommand(Job *job);
void performCommand(){
  
  //putting a job in the foreground/background should be called here 
  //and the '|', '<', etc... should be accounted for earlier in this program
  
  /*if(job->background == false){
	//run in foreground
    }else{
	//run in background
    }
  // Question: do you run the command first or do you put the job in 
  //a paricular ground before running the command?
  */
  if((strcmp("exit", qargv[0])==0) || (strcmp("quit", qargv[0])==0)){

    //and kill everything
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
  
  //this portion needs to be moved to an earlier call
  //may have the '|' checked for in another area because of the fact
  //that it must keep track of 2 jobs
  if(strchr('|', qargv[0]) == 0){
    //commandPipe();
  }if(strchr('&', qargv[0]) == 0){ //may look like (job->background == true)
    //throw to background
  }
  // check for ./ for execute function
  
  doJob(qargv, "STANDARD");
  
}

//process special commands
void doJob(char *command[], char *file){
  char* fname = NULL;
  pid_t pid;
  //foreground/background stuff
  char mode = symbolCheck("&");
  pid = fork();
  
  
}

char symbolCheck(char* symbol){
 int i=0;
  for(;qargv[i] != NULL; qargv[i]){
   if(symbol){
    if(strcmp(symbol, qargv[i])==0){	//background
     return 'b'; 
    }
   }
   else{
    if(strcmp("<", qargv[i])==0){ //read
     return 'r';
    }
    else if(strcmp(">", qargv[i])==0){//write
     return 'w'; 
    }
    else if(strcmp("|", qargv[i])==0){//pipe
      return 'p';
    }
   }
  }
  //no good
  return 0;
}




//Change Directories (cd)
void cd(const char *dir){ //--does this account for PATH and HOME if typed in? 
  //Yes -- should account for those and '..'
  
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


//Pipes -- used if there are two jobs you want to process
//these two jobs are separated by '|'
void pipeCommands(Job *job1, Job *job2){
  
  int status;
  pid_t pid_1;
  int fd1[2];
  
  pipe(fd1);
  
  
  pid_1 = fork();
  if (pid_1 == 0) {
    dup2(fd1[1],STDOUT_FILENO);
    close(fd1[0]);
    
    //would want to execute the job, but rather see what
    //type of command it is then run what it is
    //rather than going straight to an executable
    
    //I believe I should be calling the performCommand method
    //and passing in the job given
    if(execvp(job1->fileName, job1->arguments) == -1){
      //error
    }
    
    exit(0);
  }
  else {
    dup2(fd1[0], STDIN_FILENO);
    close(fd1[1]);
    waitpid(pid_1, &status, 0);
    
    //same idea as stated in the comments above for this job
    if(execvp(job2->fileName, job2->arguments) == -1){
      //error
    }
    exit(0);
  }
  
}























