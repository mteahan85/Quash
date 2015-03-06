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

#include <string.h>


#include <readline/readline.h>
#include <readline/history.h>

#include <readline/readline.h>
#include <readline/history.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <fcntl.h>


#define __USE_C99_MATH

#define BSIZE 256

pid_t shellPID;
pid_t shellPGID;
struct termios shellTmodes;
int shellTerminal;
int shellIsInteractive;




static char cmdBuf[BSIZE];
static int bufChar=0;

//hosting data/name stuff
char hostname[BSIZE];
char *directory;
char *username;


typedef struct Job{
  int background; //if the job is running in the back ground
  int alive; //if the job is alive or not
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
    directory = getcwd(NULL, 1024);
  }
  else{
    printf("Failed to make shell interactive; quitting now \n");
    exit(1);
  }
  //begLineDisplay();
  fflush(stdout);
}


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
  if (dir){
    while ((dirFiles = readdir(dir)) != NULL){ //reads files with in that directory
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
  
  
  pathType = strtok(pathSet, delim);
  value = strtok(NULL, " \n");
  
  if (setenv(pathType, value, 1) < 0){
    printf("<%s> cannot overwrite environment variables.\n", strerror(errno));
  }
}


// //Displays jobs when user calls jobs function
// void displayJobs(){
//   for (int i = 0; i < BSIZE; i++){
//     if (jobs[i].alive){
//       printf("[%d]\t%d\t%s\n",jobs[i].id,jobs[i].pid,jobs[i].command);
//     }
//   } 
// }
void execute(char** input){
  
  pid_t pid_1;
  int status;
  pid_1 = fork();
  if (pid_1 == 0) {
    // IO redirection
    
    if(execvp(input[0], input) < 0) {
      printf("That is an invalid command\n");
      exit(0);
    } 
    
  } else {
    waitpid(pid_1, &status, 0);
    if(status == 1) {
      printf( "error");
    }	
  }
  
}


void readCommand(char* in){ //parses input
  
  
  char* inputCopy = strdup(in);
  char* inputTotal = strdup(in);
  char* command;
  int qargc=0;
  char** qargv[20]; //change if too small
  for(int i = 0; i < 20; i++){
    qargv[i]=NULL; 
  }
  
  
  //break it up by spaces
  command = strtok(in," "); 
  char* arg1 = strtok(NULL," ");
  while(arg1 != NULL){
    qargv[qargc]=arg1;
    arg1=strtok(NULL," ");
    qargc++;
  }
  
  int tCount = 0;
  char** totalArgs[20];
  for(int i = 0; i < 20; i++){
    totalArgs[i]=NULL; 
  }
  char* argsT = strtok(inputTotal," "); 
  while(argsT != NULL){
    totalArgs[tCount]=argsT;
    argsT=strtok(NULL," ");
    tCount++;
  }
  
  //search for special characters
  char* isBackground = strchr(inputCopy, '&');
  char* isPipe = strchr(inputCopy, '|');
  char* isRead = strchr(inputCopy, '<');
  char* isWrite = strchr(inputCopy, '>');
  char* isCd = strstr(command, "cd");
  char* isLs = strstr(command, "ls");
  char* isExit = strstr(command, "exit");
  char* isQuit = strstr(command, "quit");
  char* isJobs = strstr(command, "jobs");
  char* isSet = strstr(command, "set");
  
  
  if(isPipe!=NULL){
    
    //background variables
    
    int spipe[2];
    char* part = strtok(inputCopy, "|");
    char* first_cmd = part;
   // printf("%s first", first_cmd);
    part = strtok(NULL, "\n");
    char* second_cmd = part;
   // printf("%s second", second_cmd);
    int status;
    pipe(spipe);
    pid_t pid1, pid2;
    pid1 = fork();
    if (pid1 == 0) {
      dup2(spipe[1], STDOUT_FILENO);
      //printf("I'm running first pipe");
      close(spipe[0]);
   
      readCommand(trimWhitespace(first_cmd));
      
     // close(spipe[1]);
      
     // printf("I finished reading first command");
      exit(0);
    } 
    pid2 = fork();
    if(pid2 == 0){
      dup2(spipe[0], STDIN_FILENO);
      close(spipe[1]);
      //printf("before wait");
     //s waitpid(pid1, &status, 0);
     // printf("after wait");
      readCommand(trimWhitespace(second_cmd));
      
      
    }
    
    close(spipe[0]);
    close(spipe[1]);
  }
  
  else if(isBackground!=NULL){
    //run in background 
    //take qargv[0] as the command
//     pid_t pidBG, sid;
//     pidBG = fork();
//     if(pidBG == 0){
//       sid = setsid();
//       if(sid < 0){
// 	printf("failed to create process group");
// 	exit(0);
//       }
//       
//       close(pipeBG[0]);
//       close(pipeBG[1]);
//       
//       close(STDIN_FILENO);
//       
//       char* bgC = strtok(inputCopy,"&"); 
//       bgC = (NULL, "\n");
//       readCommand(bgC);
//       kill(getpid(), -9);
//       exit(0);
//       
      
      
//     }
    
    
    
    
    
    
  }
  else if(isRead!=NULL){
    //read 
    int status;
    char * curInput = strdup(inputCopy);
    char * inString = strtok(inputCopy," \n");
    int inStringLen = strlen(inString);
    char * s; char * d;
    for (s=d=curInput;*d=*s;d+=(*s++!='\n')); //remove newline
    strncpy(curInput, &curInput[inStringLen + 1], strlen(curInput));
    pid_t pid_1;
    pid_1 = fork();
    if (pid_1 == 0) {
      char *e = strchr(curInput, '<');
      int index = (int)(e - curInput);
      char *inputStream = strdup(curInput);
      strncpy(inputStream, &curInput[index+2], strlen(curInput));
      int in = open(inputStream, O_RDONLY);
      dup2(in, 0);
      close(in);
      curInput[0] = '\0';
    }
  }
  else if(isWrite!=NULL){
    //write 
    char * curInput = strdup(inputCopy);
    char * inString = strtok(inputCopy," \n");
    int inStringLen = strlen(inString);
    char * s; char * d;
    for (s=d=curInput;*d=*s;d+=(*s++!='\n')); //remove newline
    strncpy(curInput, &curInput[inStringLen + 1], strlen(curInput));
    pid_t pid_1;
    pid_1 = fork();
    if (pid_1 == 0) {
      // IO redirection
      char *e = strchr(curInput, '>');
      int index = (int)(e - curInput);
      char *output = strdup(inputCopy);
      strncpy(output, &curInput[index+2], strlen(curInput));
      int fdout = open(output, O_CREAT | O_WRONLY | O_TRUNC, 0666);
      dup2(fdout, STDOUT_FILENO);
      readCommand(command);
      close(fdout);
      exit(0);
    }
    
  }
  
  else if(isCd !=NULL){
    //cd
    cd(qargv[0]);
  }
  
  else if((isExit!=NULL)||(isQuit!=NULL)){
    //quit
    exit(0);
  }
  else if(isJobs!=NULL){
    //display jobs
    //displayJobs(); 
  }
  else if(isSet!=NULL){
    // set(qargv[0]);
    set(qargv[0]);
  }
  else{
    execute(totalArgs);
   
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
  initializeShell();
  
  int flag= true;
  char* in, prompt[128];
  while(flag){
    
    snprintf(prompt,  sizeof(prompt), "[Quash %s ]$ ", getcwd(NULL,1024));
    in = readline(prompt);
    
    printf("%s\n", in);
    add_history(in);
    in = trimWhitespace(in);
    if(strlen(in)>1){	//no input    
      readCommand(in);
      
    }
    else{	//they typed something; deal with it
      flag=false;
      //
      //     begLineDisplay();
    }
    free(in);
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
