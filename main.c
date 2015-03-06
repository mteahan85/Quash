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
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#define __USE_C99_MATH

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


char* trimWhitespace(char *str){
  char* end;
  
  // Trim leading space
  while(isspace(*str)) str++;
  
  if(*str == 0)  // All spaces?
    return str;
  
  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;
  
  // Write new null terminator
  *(end+1) = 0;
  
  return str;
}




void begLineDisplay(){
  printf("%s:%s %s$ ", hostname, directory, username);
}
/*
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
}*/


//will also need to perform a special case of parsing for '|', '<', '>' case
//and for '&'
//should probably be storing this information in
//a struct job and then adding this job to the global struct Job jobs[]



//may want this to take in a job,
//and then read use the information from this job to run
//maybe method looks like this: performCommand(Job *job);


//process special commands
// void doJob(char *command[], char *file){
//   char* fname = NULL;
//   pid_t pid;
//   //foreground/background stuff
//   char mode = symbolCheck("&");
//   pid = fork();
//   
//   
// }




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
  
  
  const char* delim = "=";
  char* pathType; //either HOME or PATH
  char* value; 
  
  pathType = strtok(trimWhitespace(pathSet), delim);
  value = strtok(NULL, '\0');
  
  if (setenv(pathType, value, 1) < 0){
    printf("<%s> cannot overwrite environment variables.\n", strerror(errno));
  }
}

//Displays jobs when user calls jobs function
void displayJobs(){
  for (int i = 0; i < BSIZE; i++){
    if (jobs[i].alive){
      printf("[%d]\t%d\t%s\n",jobs[i].id,jobs[i].pid,jobs[i].command);
    }
  } 
}
void readCommand(char* input){ //parses input
  
  char* inputCopy;
  strcopy(inputCopy, input);
  char* command;
  
  //initialize
  while(qargc!=0){
    qargv[qargc] = NULL;
    qargc--;
  }
  
  //break it up by spaces
  command = strtok(input, " "); //get first token
  qargv[qargc] = strtok(input, " ");
  while(qargv[qargc] !=NULL){	//walk token through other tokens
    qargv[qargc] = strtok(NULL, " ");
    qargc++;
  }
  
  //search for special characters
  char* isBackground = strchr(inputCopy, '&');
  char* isPipe = strchr(inputCopy, '|');
  char* isRead = strchr(inputCopy, '<');
  char* isWrite = strchr(inputCopy, '>');
  char* isCd = strstr(command, "cd");
  char* isExit = strstr(command, "exit");
  char* isQuit = strstr(command, "quit");
  char* isJobs = strstr(command, "jobs");
  char* isSet = strstr(command, "set");
  
  if(isPipe!=NULL){
    char* command1 = strtok(inputCopy, "|");
    char* command2 = strtok(inputCopy, "\n");
    //pipeCommands(command1, command2);   
    
    int status;
    pid_t pid_1;
    int fd1[2];
    
    pipe(fd1);
    
    pid_1 = fork();
    if (pid_1 == 0) {
      dup2(fd1[1],STDOUT_FILENO);
      close(fd1[0]);
      readCommand(trimWhitespace(command1));
      //would want to execute the job, but rather see what
      //type of command it is then run what it is
      //rather than going straight to an executable
      
      //I believe I should be calling the performCommand method
      //and passing in the job given
      
      
      exit(0);
    }
    else {
      dup2(fd1[0], STDIN_FILENO);
      close(fd1[1]);
      waitpid(pid_1, &status, 0);
      readCommand(trimWhitespace(command2));
      exit(0);
    }
  }
  else{
    if(isBackground!=NULL){
      //run in background 
      //take qargv[0] as the command
    }
    else if(isRead!=NULL){
      //read  
    }
    else if(isWrite!=NULL){
      //write 
    }
    else if(isCd !=NULL){
      //cd
    }
    else if((isExit!=NULL)||(isQuit!=NULL)){
      //quit
      exit(0);
    }
    else if(isJobs!=NULL){
      //display jobs
      displayJobs(); 
    }
    else if(isSet!=NULL){
      //do set thing
      
    }
    
  }
}
int main(int argc, char **argv, char **envp)
{
  //let's just designate things to individual files, so they look contained and pretty
  
  printf("##############################################\n");
  printf("#                   Quash                    #\n");
  printf("# Written by: Nicole Maneth and Megan Teahan #\n");
  printf("##############################################\n");
  
  //initialize shell
  //initializeShell();
  
  while(1){
    char* in = readLine("Quash$");
    add_history(in);
    char* cleanIn = trimWhitespace(in);
    if(cleanIn=='\n'){	//no input    
      //     begLineDisplay();
    }
    else{	//they typed something; deal with it
      readCommand(cleanIn);
      //
      //     begLineDisplay();
    }
  } 
  
}
//Pipes -- used if there are two jobs you want to process
//these two jobs are separated by '|'
// void pipeCommands(char* job1, char* job2){
//   
//   int status;
//   pid_t pid_1;
//   int fd1[2];
//   
//   pipe(fd1);
//   
//   pid_1 = fork();
//   if (pid_1 == 0) {
//     dup2(fd1[1],STDOUT_FILENO);
//     close(fd1[0]);
//     readCommand(trimWhitespace(job1));
//     //would want to execute the job, but rather see what
//     //type of command it is then run what it is
//     //rather than going straight to an executable
//     
//     //I believe I should be calling the performCommand method
//     //and passing in the job given
//     
//     
//     exit(0);
//   }
//   else {
//     dup2(fd1[0], STDIN_FILENO);
//     close(fd1[1]);
//     waitpid(pid_1, &status, 0);
//     readCommand(trimWhitespace(job2));
//     exit(0);
//   }
//   
// }
