#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <sys/wait.h>

#define BSIZE 256

/* A process is a single process.  */
typedef struct process
{
  struct process *next;       /* next process in pipeline */
  char **argv;                /* for exec */
  pid_t pid;                  /* process ID */
  char completed;             /* true if process has completed */
  char stopped;               /* true if process has stopped */
  int status;                 /* reported status value */
} process;
/* A job is a pipeline of processes.  */
typedef struct Job
{
  struct Job *next;           /* next active job */
  char *command;              /* command line, used for messages */
  process *first_process;     /* list of processes in this job */
  pid_t pid;                 /* process group ID */
  char notified;              /* true if user told about stopped job */
  struct termios tmodes;      /* saved terminal modes */
  int stdin, stdout, stderr;  /* standard i/o channels */
  bool alive;
  bool background;
  int id;
  
} job;

/* The active jobs are linked into a list.  This is its head.   */
Job *first_job = NULL;

/* Find the active job with the indicated pgid.  */
job *
find_job (pid_t pgid)
{
  job *j;

  for (j = first_job; j; j = j->next)
    if (j->pid == pgid)
      return j;
  return NULL;
}
/* Return true if all processes in the job have stopped or completed.  */
int
job_is_stopped (job *j)
{
  process *p;

  for (p = j->first_process; p; p = p->next)
    if (!p->completed && !p->stopped)
      return 0;
  return 1;
}
/* Return true if all processes in the job have completed.  */
int
job_is_completed (job *j)
{
  process *p;

  for (p = j->first_process; p; p = ppid_t shell_pgid;
struct termios shell_tmodes;
int shell_terminal;->next)
    if (!p->completed)
      return 0;
  return 1;
}

/*typedef struct{
  bool background; 
  bool alive;
  int id;
  pid_t pid;
  char * command;
  
} Job;
*/





int main(int argc, char **argv, char **envp)
{	//let's just designate things to individual files, so they look contained and pretty

  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  bool gettingInput;
  if (isatty(fileno(stdin)))
    gettingInput = false;
  else
    gettingInput = true;
  
  
  if(!gettingInput){
    //make pretty welcome thing here
    printf("##############################################\n");
    printf("#                   Quash                    #\n");
    printf("# Written by: Nicole Maneth and Megan Teahan #\n");
    printf("##############################################\n");
       
  }
  //will have to initalize (A LOT) of things
  //hosting data/name stuff
  char hostname[BSIZE];
  gethostname(hostname,BSIZE);
  char *directory = getDirectory();
  char *username = getenv("USER");
  
  //command stuff
  char command[BSIZE];
  size_t length = BSIZE;
  
  //job stuff
  Job jobs[BSIZE];
  for( int i=0; i<BSIZE; i++){
    jobs[i].alive=false;
    jobs[i].command = malloc(10);
  }
  int numJobs = 0;
  
  //pid stuff
  pid_t returnPid;
  int pid;
  int status;
  
  //pipes stuff
  
  
  
  //have loop to run actual commands on
  while(1){
    if(!gettingInput)
      printf("%s:%s %s$ ", hostname, directory, username);
    char *command = malloc((int)length);
    if(getline(&command, &length, stdin)>0){//command was made
      for(int i=0; i<BSIZE; i++){
	
	if(jobs[i].alive){//check on jobs
	  returnPid = waitpid(jobs[i].pid, &status, WNOHANG);  //get pid
	  if(returnPid<0){
	    printf("failed");
	    jobs[i].alive=false;
	  }
	  else if(returnPid == jobs[i].pid){
	    printf("finished");
	    jobs[i].alive = false;
	  }
	  else
	    continue;
	}
	
      }
      
      
      //see what the command is
      if((string)argv[0] =="exit" || (string)argv[0] =="quit"){
	exit(0);
      }
      if((string)argv[0] =="set"){
	set();
      }
      if((string)argv[0] =="cd"){
	cd(argv[1]);
      }
      if((string)argv[0] =="jobs"){
	jobs();
      }
      if((string)argv[0] =="ls"){
	ls();
      }
      
      
    }
    
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