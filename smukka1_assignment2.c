#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>




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
#define KILL_CMD "kill"
#define Filter "|"


FILE *fp; // file struct for stdin
char **tokens;
char **split_tokens;

char *line,*input_fname,*output_fname,*fline,*tmpline,*input_fname1,*output_fname1;
pid_t pid;
int token_count = 0;
int status,in_rd,out_rd,fd0,fd1,filter_cmd,bg_ind,in_rd1,out_rd1,fds0,fds1,split_act_count;

void initialize()
{

	// allocate space for the whole line
	assert( (line = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);

	// allocate space for individual tokens
	assert( (tokens = malloc(sizeof(char*)*MAX_TOKENS)) != NULL);
 
 // allocate space for individual tokens
	assert( (split_tokens = malloc(sizeof(char*)*MAX_TOKENS)) != NULL);
 
 
	// allocate space for the whole line
	assert( (fline = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);
 
 
	// allocate space for the whole line
	assert( (tmpline = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);

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

void killProcess(char **tokens)
{

	pid_t pid_p;
  
	if(tokens[1]!=NULL)
	{
		pid_p=atoi(tokens[1]);
		kill(pid_p,SIGKILL);
    
	}else{
		printf("please provide process ID\n");
	}

}

void sigint(){
  
	signal(SIGINT,sigint);
   //printf("\n intererupt");
}

//Function to execute commands
void execCmnds(char **tokens){

  int ret;
  signal(SIGINT,sigint); 
  pid=fork();   //create a child process
    
	//for normal commands
	if(bg_ind ==0 )
	{	
     //printf("inside exce comnads function");
		if(pid<0)
		{
			perror("fork failed");
		}else if(pid==0)
		{ 
		//child executes
			if(in_rd==1)
			{
				fd0=open(input_fname,O_RDONLY);
				dup2(fd0,0);
				in_rd=0;
				close(fd0);
			}
			//printf("\n 2. this is %d, Out file name is %s",out_rd,output_fname);
			if(out_rd==1){
				
				tokens[token_count -1] = NULL;
				
				//printf("\n\ntoken value is %s\n",tokens[token_count -1]);
				fd1=creat(output_fname,0644);
				dup2(fd1,1);
				out_rd=0;
				close(fd1);
			} 
				
			execvp(tokens[0],tokens);
			kill(pid,SIGINT);
				perror("exec failed");
				exit(1);
			
		} else{
      //parent executes
			ret=waitpid(pid,&status,0);
			//signal(SIGINT,sigint);
			if(ret<0)
			{
				perror("waitpid failed");
			}
		}
	}else //for background commands
	{
     //printf("in background jobs\n");
     //printf("\n %s",tokens[token_count -1] );
		//tokens[token_count -1] = NULL;
		
		if(pid<0)
		{
			perror("fork failed");
		}else if(pid==0)
		{ 
       //child executes
	   signal(SIGINT,SIG_IGN);
			if(in_rd==1)
			{
				fd0=open(input_fname,O_RDONLY);
				dup2(fd0,0);
				in_rd=0;
				close(fd0);
			}
			//printf("\n 2. this is %d, Out file name is %s",out_rd,output_fname);
			if(out_rd==1){
				
				tokens[token_count -1] = NULL;
				
				//printf("\n\ntoken value is %s\n",tokens[token_count -1]);
				fd1=creat(output_fname,0644);
				dup2(fd1,1);
				out_rd=0;
				close(fd1);
			} 
			  execvp(tokens[0],tokens);
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

int filter_tokenize (char * string, int pos)
{
	
	int size = MAX_TOKENS;
	char *this_token;
  in_rd1=0;out_rd1=0;
	memset(split_tokens,'\0', 10*sizeof(char));	
	//to make tokens null for every command
	 int split_token_count = 0;
	split_act_count = 0;	
 
	while ( (this_token = strsep( &string, " \t\v\f\n\r")) != NULL) 
	{

		if (*this_token == '\0') continue;
		
   	if(*this_token == '<'){
			in_rd1=1;
			continue;
		}
 	  if(*this_token == '>'){
			out_rd1=1;
			continue;
		}
   	if(in_rd1==1){
			input_fname1=this_token;
		}
		
		if(out_rd1==1){
			output_fname1=this_token;
			//printf("\n 1. this is %d, Out file name is %s",out_rd,output_fname);
			//continue;			
		}
		
		if (split_token_count >= pos ){
			
			if(*this_token=='|'){
				return split_token_count;
				break;
			}	
			split_tokens[split_act_count] = this_token;
			split_act_count++;
		}
		
		split_token_count++;
		// if there are more tokens than space ,reallocate more space
		if(split_token_count >= size)
		{
			size*=2;

			assert ( (tokens = realloc(split_tokens, sizeof(char*) * size)) != NULL);
		}
	}
	
	return split_token_count;
}

void filterCmnds(char **tokens){

	int filter[10],filtercount=0;
	int fd[2],fd1[2],fd2[2];
	pid_t pid1,pid2;
	int i,j,pnum =0;
	char **newArray[100];
	memset(filter,0, 10*sizeof(int));
 
		signal(SIGINT,sigint);
  for(i=0; i<token_count; i++){
 
		if(strcmp(tokens[i],Filter)==0){
			filter[filtercount]=i;
			filtercount++;
      //printf("\n getting in tokenize %d",filter[filtercount]);
		}	
	}
	if(pipe(fd1)==-1){
		perror("pipe1");
	}
	
	if(pipe(fd2)==-1){
		perror("pipe2");
	}
	//printf("\n getting in tokenize");	
	int n=0,l=0;
  tmpline=strdup(fline);
	for (i=0; i<= filtercount; i++){
 		fline=strdup(tmpline);
		n=filter_tokenize(fline,l);
    //printf("\n value of n is %d",n); 
		l=n+1;
		if(pnum==0){
			pnum=1;
		} else {
			pnum=0;
		}    
    if(i< filtercount){ 
   
			pid2=fork();
   
			if(pid2==0){
      
        	if(in_rd1==1)
			{
				fds0=open(input_fname1,O_RDONLY);
				dup2(fds0,0);
				in_rd1=0;
				close(fds0);
			}
			//printf("\n 2. this is %d, Out file name is %s",out_rd,output_fname); 
      
				if (pnum==0){
					
					// If pnum = 0 write to first pipe and read from second pipe
					//pipe 1
					close(1);  /* close normal stdout (fd = 1) */
					dup2(fd1[1], 1);   /* make stdout same as fds[1] */
					close(fd1[0]); /* we don't need the read end -- fds[0] */
                                   
					//pipe 2
					close(0);   /* close normal stdin (fd = 0)*/
					dup2(fd2[0],0);   /* make stdin same as fds[0] */
					close(fd2[1]); /* we don't need the write end -- fds[1]*/
				} else{
					// If pnum = 0 write to first pipe and read from second pipe
					//pipe 2
					close(1);  /* close normal stdout (fd = 1) */
					dup2(fd2[1], 1);   /* make stdout same as fds[1] */
					close(fd2[0]); /* we don't need the read end -- fds[0] */
					//pipe 1
					close(0);   /* close normal stdin (fd = 0)*/
					dup2(fd1[0],0);   /* make stdin same as fds[0] */
					close(fd1[1]); /* we don't need the write end -- fds[1]*/
					
				}

				if(execvp(split_tokens[0],split_tokens)<0){	
					perror("exec failed");
          exit(1); 
				}
			}
			
			if(pid2>0){
				int res=waitpid(pid2,&status,0);
				//printf("\n iteration is %d",i);
				if(pnum==1){
				close(fd2[1]);
				
				if(pipe(fd1)==-1){
						perror("pipe2");
					}
				} else {
				close(fd1[1]);
				
				if(pipe(fd2)==-1){
						perror("pipe2");
					}
				}
					}
				} else {
					pid_t pid3=fork();
					if(pid3==0){ 
          	if(out_rd1==1){
				printf("\n value of split token is %d",split_act_count);
				split_tokens[split_act_count -1] = NULL;
				
				//printf("\n\ntoken value is %s\n",tokens[token_count -1]);
				fds1=creat(output_fname1,0644);
				dup2(fds1,1);
				out_rd1=0;
				close(fds1);
			}
					if (pnum==0){
						// If pnum = 0 write to first pipe and read from second pipe
						//pipe 2
						close(0);   /* close normal stdin (fd = 0)*/
						dup2(fd2[0],0);   /* make stdin same as fds[0] */
						close(fd2[1]); /* we don't need the write end -- fds[1]*/
					} else if (pnum==1){
						// If pnum = 0 write to first pipe and read from second pipe
						//pipe 1
						close(0);   /* close normal stdin (fd = 0)*/
						dup2(fd1[0],0);   /* make stdin same as fds[0] */
						close(fd1[1]); /* we don't need the write end -- fds[1]*/
					}
					
					if(execvp(split_tokens[0],split_tokens)<0){	
						perror("exec failed");
            exit(1);
					}
				}
        if(pid3>0){
          int res2=waitpid(pid3,&status,0);
        }
		}
	}
  
	close(fd1[0]); 
    close(fd1[1]);
	close(fd2[0]); 
    close(fd2[1]);	
}

void tokenize (char * string)
{
	
	int size = MAX_TOKENS;
	char *this_token;
	in_rd = 0;
	out_rd = 0,bg_ind=0,filter_cmd=0;
	
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
    
		if(*this_token == '<'){
			in_rd=1;
			continue;
		}
    
		if(*this_token == '&'){
			bg_ind=1;
			continue;
		}
		
		if(*this_token == '>'){
			out_rd=1;
			continue;
		}
   
		if(*this_token == '|'){
			filter_cmd=1;
		}
		
		if(in_rd==1){
			input_fname=this_token;
		}
		
		if(out_rd==1){
			output_fname=this_token;
			//printf("\n 1. this is %d, Out file name is %s",out_rd,output_fname);
			//continue;			
		} 
		
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
  }else if(strcmp(tokens[0],KILL_CMD)==0){
    killProcess(tokens);
  }else if(filter_cmd==1){
    filterCmnds(tokens);
  }else{
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
    fline=strdup(line);   
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