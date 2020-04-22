#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>




//limits
#define MAX_TOKENS 100
#define MAX_STRING_LEN 100

size_t MAX_LINE_LEN = 10000;


// builtin commands
#define EXIT_STR "exit"
#define EXIT_CMD 0
#define UNKNOWN_CMD 99
#define SKIP_STR ""
#define BG_IND "&"
#define LIST_JOBS "listjobs"
#define FORE_GROUND "fg"


FILE *fp; // file struct for stdin
char **tokens;
char *line;
pid_t pid;
int token_count = 0;
int status;

void initialize()
{

	// allocate space for the whole line
	assert( (line = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);

	// allocate space for individual tokens
	assert( (tokens = malloc(sizeof(char*)*MAX_TOKENS)) != NULL);

	// open stdin as a file pointer
	assert( (fp = fdopen(STDIN_FILENO, "r")) != NULL);

}

//Data Structure to hold Background jobs
struct backgroundJobs{
	int pId;
	char *cmd;
	struct backgroundJobs *next;
}bgjob;

struct backgroundJobs *head=NULL;

// Function to print background jobs
void listJobs(char **tokens)
	{
  int bgstatus;
	struct backgroundJobs *list=head;
  
  if(list==NULL){
    printf("\n There are no background Processes\n");
  }
	while(list!=NULL)
	{
		//printf("\n %s with PID %d status %s\n",list->cmd,list->pId,list->status);
    bgstatus=waitpid(list->pId,&status,WNOHANG); // to get the status of the background jobs
    if(bgstatus==0){
      printf("\n %s with PID %d status:Running\n",list->cmd,list->pId);
     }else{
      printf("\n %s with PID %d status:Completed\n",list->cmd,list->pId);
     }
    //printf("\n %d is the test stat\n",test);
		list=list->next;
   
	} 
}

//Function to add background jobs to the data structure defined 
void addJobs(int id,char **tokens,int status )	
	{
	
	//printf("\ninside add jobs %s\n", tokens[0]);
	
	struct backgroundJobs *link=(struct backgroundJobs*)malloc(sizeof( struct backgroundJobs ));
	
	//printf("\n inside add jobs %s\n",status_text);
	link->pId=id;
	link->cmd=tokens[0];
	link->next=head;
  
	head=link;
	}

//Function to make a process foreground
void makeForeground(char **tokens)
{
    int ret1;
    pid_t pid_fg;
    if(tokens[1]!=NULL)
    {
      pid_fg=atoi(tokens[1]);
      //printf("in fg %d, %s\n",pid_fg,tokens[1]);
      ret1=waitpid(pid_fg,&status,0);
    
      if(ret1<0)
			{
				perror("waitpid failed");
			}
   }else{
     printf("please provide process ID\n");
   }

}

//Function to execute commands
void execCmnds(char **tokens){

  int ret;
  
    pid=fork();   //create a child process
    
  //for normal commands
	if(strcmp(tokens[token_count -1],BG_IND) !=0)
	{	
		if(pid<0)
		{
			perror("fork failed");
		}else if(pid==0)
		{ 
       //child executes
			if((execvp(tokens[0],tokens))<0);
        perror("exec failed");
        exit(1);
		} else{
      //parent executes
			ret=waitpid(pid,&status,0);
			if(ret<0)
			{
				perror("waitpid failed");
			}
		}
	}
	else //for background commands
	{
		tokens[token_count -1] = NULL;
		
		if(pid<0)
		{
			perror("fork failed");
		}else if(pid==0)
		{ 
       //child executes
			if((execvp(tokens[0],tokens))<0);
        perror("exec failed");
        exit(1);
		} else
		{
      //parent executes
			addJobs(pid,tokens,status);  //adds backround jobs to structure
			ret=waitpid(pid,&status,WNOHANG);
			if(ret<0)
			{
				perror("waitpid failed");
			}
		}
	}
}

void tokenize (char * string)
{
	
	int size = MAX_TOKENS;
	char *this_token;
 //to make tokens null for every command
	if (token_count > 0) 
	{
		for(int j=0;j< token_count;j++)
		{
			tokens[j]=NULL;
		}
	} 
   
	token_count = 0; 
 
  
  //**tokens=NULL;
  
	while ( (this_token = strsep( &string, " \t\v\f\n\r")) != NULL) 
	{

		if (*this_token == '\0') continue;

		tokens[token_count] = this_token;
   
		token_count++;
		// if there are more tokens than space ,reallocate more space
		if(token_count >= size)
		{
			size*=2;

			assert ( (tokens = realloc(tokens, sizeof(char*) * size)) != NULL);
		}
	}
	if(strcmp(tokens[0],LIST_JOBS)==0)
	{
		listJobs(tokens);
	}else if(strcmp(tokens[0],FORE_GROUND)==0)
	{
		makeForeground(tokens);
	}else if(strcmp(tokens[0],EXIT_STR)==0){
  }else
	{
		execCmnds(tokens);
	}
}


void read_command()
{
	int length;
	// getline will reallocate if input exceeds max length
	assert( getline(&line, &MAX_LINE_LEN, fp) > -1);

	//printf("Shell read this line: %s\n", line);
	length = strlen(line);
	//printf("%d\n",length);
	if(length>1)  //to handle segmentation error when we press enter
	{
   
		tokenize(line);
	}else
  {
     tokens[0] = SKIP_STR;
  }
}



int run_command() {

	if (strcmp( tokens[0], EXIT_STR ) == 0)
		return EXIT_CMD;

	return UNKNOWN_CMD;
}

int main()
{
	//initialize();

	do {
		printf("sh550> ");
    initialize();
		read_command();

	} while( run_command() != EXIT_CMD );

	return 0;
}