#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<fcntl.h>
#include "tokenizer.h"

//#define DEBUG
#define INPUT_SIZE 1024

pid_t childPid = 0;

pid_t childPid1 = 0;

pid_t childPid2 = 0;

void executeShell(int timeout);

void writeToStdout(char *text);

void alarmHandler(int sig);

void sigintHandler(int sig);

char **getCommandFromInput();

void registerSignalHandlers();

void killChildProcess();

int getRedirectionCount(char **command, int ind);

void redirectionSTDOUTtoFile(char* filename);

void redirectionSTDINtoFile(char* filename);

char** createSubArray(char** array, int start, int end);

void redirectionSTDOUTtoFile(char* filename);

void redirectionSTDINtoFile(char* filename);

char **redirectionPipesWriterProcess(char **commandArray1);

char **redirectionPipesReaderProcess(char **commandArray2);

int main(int argc, char **argv) {
    registerSignalHandlers();

    int timeout = 0;
    //if (argc == 2) {
        //timeout = atoi(argv[1]);
    //}

    if (timeout < 0) {
        writeToStdout("Invalid input detected. Ignoring timeout value.\n");
        timeout = 0;
    }

    while (1) {
        executeShell(timeout);
    }

    return 0;
}

/* Sends SIGKILL signal to a child process.
 * Error checks for kill system call failure and exits program if
 * there is an error */
void killChildProcess() {
    if (kill(childPid, SIGKILL) == -1) {
        perror("Error in kill");
        exit(EXIT_FAILURE);
    }
}

/* Signal handler for SIGALRM. Catches SIGALRM signal and
 * kills the child process if it exists and is still executing.
 * It then prints out penn-shredder's catchphrase to standard output */
void alarmHandler(int sig) {

}

/* Signal handler for SIGINT. Catches SIGINT signal (e.g. Ctrl + C) and
 * kills the child process if it exists and is executing. Does not
 * do anything to the parent process and its execution */
void sigintHandler(int sig) {
    if (childPid != 0) {
        killChildProcess();
    }
}


/* Registers SIGALRM and SIGINT handlers with corresponding functions.
 * Error checks for signal system call failure and exits program if
 * there is an error */
void registerSignalHandlers() {
    if (signal(SIGINT, sigintHandler) == SIG_ERR) {
        perror("Error in signal");
        exit(EXIT_FAILURE);
    }
    ///for part b
    // if(signal(SIGALARM, alarmHandler) == SIG_ERR){ printf("Unable to catch SIGINT\n"); exit(EXIT_FAILURE)};
}

/* Prints the shell prompt and waits for input from user.
 * Takes timeout as an argument and starts an alarm of that timeout period
 * if there is a valid command. It then creates a child process which
 * executes the command with its arguments.
 *
 * The parent process waits for the child. On unsuccessful completion,
 * it exits the shell. */
void executeShell(int timeout) {
    
    int status;
    char minishell[] = "penn-sh# ";	
    writeToStdout(minishell);
    int commandCt =0;
    int pipeIndex =0;
    int pipeCt =0;

	char **command = getCommandFromInput();

    #ifdef DEBUG
	printf("input string first in execshell is %s\n", command[0]);
    printf("input string second in execshell is %s\n", command[1]);
    printf("input string third in execshell is %s\n", command[2]);	
	#endif
    
    if (command[0] != NULL) {	  
      
//cehck for pipe symbol       
        while(command[commandCt] != NULL){
	        if (strchr(command[commandCt] ,'|' ) != NULL) {
		    pipeIndex = commandCt;
		    pipeCt ++;       
	        }
    commandCt++;

        }
//printf("pipe count  line 145 %d\n", pipeCt);

//continue with project 2a logic if there is no pipe symbol	
    if (pipeCt ==0) {
        childPid = fork();

        if (childPid < 0) {
            perror("Error in creating child process");
            free(command);
            exit(EXIT_FAILURE);
        }
       
       if (childPid == 0) {
           //initialize the index of command array and args array
           int index = 0;
           int argsIndex = 0;

           while(command[index] !=NULL){

            if(strchr(command[index] ,'>' )!= NULL) {
                int outRedirectCt = getRedirectionCount(command, index);
                if( argsIndex == 0){ argsIndex = index;}

                if (outRedirectCt >1){
                   free(command);
                    perror("Invalid number of redirections");
                    return;
                }              
                redirectionSTDOUTtoFile(command[index+1]);
            }

            if(strchr(command[index] ,'<' )!= NULL){
                int outRedirectCt = getRedirectionCount(command, index);
                if( argsIndex == 0){ argsIndex = index;}
                if (outRedirectCt >1){
                    free(command);
                    perror("Invalid number of redirections");
                    return;
                }
                redirectionSTDINtoFile(command[index+1]);
            
            }
            index++;

           }

           char * args[index] ;

           if (argsIndex != 0){
           for(int i=0; i< argsIndex; i++){
            args[i] = command[i];

           }
           args[argsIndex] = '\0';
       
           }

           else {
            for (int j =0; j<index; j++){
                args[j] = command[j];
            }
            args[index] = '\0';
           }
          
            if (execvp(command[0], args) == -1) {
                perror("Error in execvp");
                free(command);
                exit(EXIT_FAILURE);
            }
        } else {
            do {
                if (wait(&status) == -1) {
                    perror("Error in child process termination");
                   free(command);
                    exit(EXIT_FAILURE);
                }
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
            childPid = 0;
            free(command);   
  }

  if (pipeCt == 1) {
    int startInd = 0;
    int endInd = pipeIndex-1;
    int ind =0;
    int wordLen =0;

//printf("line 230 check, start is %d, endInd is %d\n", startInd, pipeIndex-1);
     
    char** commandArray1 = calloc(INPUT_SIZE, sizeof(char*));
    if (commandArray1 == NULL) { 
        perror("could not allocate memory");
        free(commandArray1);
    }
    	while (startInd <= endInd){
		wordLen = strlen(command[startInd]);
        commandArray1[ind] = calloc(wordLen+1, sizeof(char));
		for (int i =0; i< wordLen; i++){
		commandArray1[ind][i] = command[startInd][i];}
		commandArray1[ind][wordLen] = '\0';
		startInd++;
        ind++;
	}

    startInd = pipeIndex+1;
    endInd = commandCt-1;
    ind =0;
    wordLen =0;
//printf("lne 255 for commandArray2, start index is %d, end index is %d \n", startInd, endInd);

    char** commandArray2 = calloc(INPUT_SIZE, sizeof(char*));
    if (commandArray2 == NULL) { 
        perror("could not allocate memory");
        free(commandArray2);
    }
    	while (startInd <= endInd){
		wordLen = strlen(command[startInd]);
        commandArray2[ind] = calloc(wordLen+1, sizeof(char));
		for (int i =0; i< wordLen; i++){
		commandArray2[ind][i] = command[startInd][i];}
		commandArray2[ind][wordLen] = '\0';

//printf("line 269 check, commandarray2: %s\n", commandArray2[ind]);
		startInd++;
        ind++;
	}

  	 //char** commandArray2 = createSubArray(command, startInd, commandCt);
 
//printf("commandarray2 entry1 line 276 is %s\n", commandArray2[0]);
//printf("childPid: %d, childPid1: %d, childPid2:%d\n", childPid, childPid1, childPid2);

  	  int fd[2];
  	  pipe(fd);
      childPid1 = fork();
 // printf(" line 288 childPid: %d, childPid1: %d, childPid2:%d\n", childPid, childPid1, childPid2);	
       if (childPid1 < 0) {
            perror("Error in creating child process");
            free(command);
            exit(EXIT_FAILURE);
        } 	  
        if (childPid1 == 0) {
 //printf(" line 295 childPid: %d, childPid1: %d, childPid2:%d\n", childPid, childPid1, childPid2);
 
           char** args1 = redirectionPipesWriterProcess(commandArray1);

   //printf(" line 305 childPid: %d, childPid1: %d, childPid2:%d\n", childPid, childPid1, childPid2);   
     // printf("line 294 check, commandarray1[0]:%s, args1[0]:%s, args[1]: %s, args[2]: %s\n", commandArray1[0], args1[0], args1[1], args1[2]);          
           close(fd[0]);        //close read end           
           dup2(fd[1], STDOUT_FILENO);          //pipe output redirection
           close(fd[1]);                    //close duplicate write end

           if (execvp(args1[0], args1) == -1) {
                perror("Error in execvp");
                free(commandArray1);
                free(command);
                exit(EXIT_FAILURE);
           }
        }  

 childPid2 = fork();
      if (childPid2 < 0) {
            perror("Error in creating child process");
            free(command);
            exit(EXIT_FAILURE);
        }

        if (childPid2 == 0) {
            char** args2 = redirectionPipesReaderProcess(commandArray2);
 //printf("line 316 check, commandarray2[0]:%s, commandarray2[1]:%s, commandarray2[2]:%s, args2[0]:%s, args2[1]: %s, args[2]: %s\n", commandArray2[0], commandArray2[1],commandArray2[2],args2[0], args2[1], args2[2]);              
	        close (fd[1]);            //close write end of pipe
            dup2(fd[0], STDIN_FILENO);
            close(fd[0]);
         
            if (execvp(args2[0], args2) == -1) {
                perror("Error in execvp");
                free(commandArray1);
                free(commandArray2);
                free(command);
                exit(EXIT_FAILURE);
            }
         } 

        //close both pipe ends in the parent process
         else {
              close(fd[0]);
              close(fd[1]);
          while (waitpid(-1, &status, 0) > 0) {
            // do nothing
            }
	    }
            childPid1 = 0;
            childPid2 =0;
            free(commandArray1);
            free(commandArray2);
            free(command);
  }
	 
    } else printf("the command is null");
}

char **redirectionPipesWriterProcess(char **commandArray1){
 
    int index = 0;
    int argsIndex =0;
    int ind =0;
    int strLen =0;
    char** args1 = calloc(INPUT_SIZE, sizeof(char*));

    while(commandArray1[index] !=NULL){
        if(strchr(commandArray1[index] ,'>' )!= NULL) {
            free(commandArray1);       
            perror("Invalid for PipeWriter Process");
            exit(0);
        }

        if(strchr(commandArray1[index] ,'<' )!= NULL){
            int outRedirectCt = getRedirectionCount(commandArray1, index);
                if( argsIndex == 0){ argsIndex = index;}
                if (outRedirectCt >1){
                    free(commandArray1);                  
                    perror("Invalid number of redirections");
                    exit(0);
                }
                redirectionSTDINtoFile(commandArray1[index+1]);
            }

            index++;
        }

        if (argsIndex != 0){
        while(ind!= argsIndex) {
            strLen = strlen(commandArray1[ind]);
            args1[ind] = calloc(strLen+1, sizeof(char));
                for (int i =0; i <strLen; i++){
            args1[ind][i] = commandArray1[ind][i];}
                args1[ind][strLen] = '\0';
                ind++;
                }  
        args1[argsIndex] = NULL;
        return args1;
        }

        if (argsIndex == 0){
        while(commandArray1[ind] != NULL) {
            strLen = strlen(commandArray1[ind]);
            args1[ind] = calloc(strLen+1, sizeof(char));
                for (int i =0; i <strLen; i++){
            args1[ind][i] = commandArray1[ind][i];}
                args1[ind][strLen] = '\0';
                ind++;
                }
        args1[ind] = NULL;
         } return args1;
        
}

char **redirectionPipesReaderProcess(char **commandArray2){
    int index = 0;
    int argsIndex =0;
    int ind =0;
    int strLen =0;
    char** args2 = calloc(INPUT_SIZE, sizeof(char*));

    while(commandArray2[index] !=NULL){
    	  if(strchr(commandArray2[index] ,'>' )!= NULL) {  	
          int outRedirectCt = getRedirectionCount(commandArray2, index);
               if( argsIndex == 0){ argsIndex = index;}
               if (outRedirectCt >1){                
                  free(commandArray2);                 
                    perror("Invalid number of redirections");
                  exit(0);      		 
                }
                redirectionSTDOUTtoFile(commandArray2[index+1]);
          }

        if(strchr(commandArray2[index] ,'<' )!= NULL){                    
                free(commandArray2);              
                perror("Invalid for pipe Reader Process");
                exit(0);              
            }
            index++;
        }

        if (argsIndex != 0){
        while(ind!= argsIndex) {
            strLen = strlen(commandArray2[ind]);
            args2[ind] = calloc(strLen+1, sizeof(char));
                for (int i =0; i <strLen; i++){
            args2[ind][i] = commandArray2[ind][i];}
                args2[ind][strLen] = '\0';
                ind++;
                }
        args2[argsIndex] = NULL;
        return args2;
        }

      if (argsIndex == 0){
        while(commandArray2[ind] != NULL) {
            strLen = strlen(commandArray2[ind]);
            args2[ind] = calloc(strLen+1, sizeof(char));
                for (int i =0; i <strLen; i++){
            args2[ind][i] = commandArray2[ind][i];}
                args2[ind][strLen] = '\0';
   // printf("line 454 args2 %s\n", args2[ind]);
                ind++;
                }
        args2[ind] = NULL;
         } return args2;      
}


char** createSubArray(char** array, int start, int end){
//printf("line 421 --createSubArray function first line check");

	char** subArray = calloc(INPUT_SIZE, sizeof(char*));
    if (subArray == NULL) { 
        perror("could not allocate memory");
        free(subArray);
    }

	//char* word = NULL;
	int startInd = start;
	int endInd = end;
	int ind =0;
    int wordLen =0;

//printf("line435 check");
	while (startInd <= endInd){
		wordLen = strlen(array[startInd]);
        subArray[ind] = calloc(wordLen+1, sizeof(char));
		for (int i =0; i< wordLen; i++){
		subArray[ind][i] = array[startInd][i];}
		subArray[ind][wordLen] = '\0';
		startInd++;
        ind++;
	}
	return subArray;
}


int getRedirectionCount(char **command, int ind){
int count =0;
int i =0;

while(command[i] != NULL){
    if((strcmp(command[i] ,command[ind])) == 0){
        count++;}
    i++;
    }
//printf("count line 235 is %d\n", count);
return count;
}

void redirectionSTDOUTtoFile(char* filename){
//printf("line 204 filename %s\n", filename);
  int new_stdout = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  dup2(new_stdout, STDOUT_FILENO);
  close(new_stdout);
}

void redirectionSTDINtoFile(char* filename){
  int new_stdin = open(filename, O_RDONLY );
//printf("line 216 check, value of new_stdin %d\n", new_stdin);
  if (new_stdin == -1){
   perror("Invalid input file");
   
   exit(0);
  }
  dup2(new_stdin, STDIN_FILENO);
  close(new_stdin);
}


/* Writes particular text to standard output */
void writeToStdout(char *text) {
    if (write(STDOUT_FILENO, text, strlen(text)) == -1) {
        perror("Error in write");
        exit(EXIT_FAILURE);
    }
}

/* Reads input from standard input till it reaches a new line character.
 * Checks if EOF (Ctrl + D) is being read and exits penn-shredder if that is the case
 * Otherwise, it checks for a valid input and adds the characters to an input buffer.
 *
 * From this input buffer, the first 1023 characters (if more than 1023) or the whole
 * buffer are assigned to command and returned. An \0 is appended to the command so
 * that it is null terminated */
  
char **getCommandFromInput() {
  
    TOKENIZER  *strOfTokens;
	
    char *tok = NULL;
    
    char **commandFromInput = calloc(INPUT_SIZE, sizeof(char*));
    if (commandFromInput == NULL){
		perror("could not allocate memory");
		free(commandFromInput);
	}

    char *input = (char*) calloc(INPUT_SIZE, sizeof(char));

	if (input == NULL){
		perror("could not allocate memory");
		free(input);
	}
	
	int numbytesread;
	
	int index =0;
	
    int length_string = 0;

	//read from standard input into input char string upto 1024 bytes
	numbytesread = read(STDIN_FILENO, input, INPUT_SIZE);

	#ifdef DEBUG
    printf("numbytesread is  %d\n", numbytesread);
    //printf("input string is %s\n", input);
    #endif

	//if error occured in reading then exit with an error message
	if (numbytesread == -1){
		free(input);
        free(commandFromInput);
		perror("Error in read");
		exit(EXIT_FAILURE);
	}
	
	//if ^D is entered, exit the entire program successfully
	if (numbytesread == 0){ 
		free(input);
        free(commandFromInput);
		exit(0);}
		
	//if (numbytesread >0)
    while (numbytesread >0){
        if(numbytesread <=1){
           // free(input);
            break;
        }
        input[numbytesread-1] = '\0';        /* remove trailing \n */
        break;
    }
	strOfTokens = init_tokenizer(input);
    while ((tok = get_next_token(strOfTokens) )!=NULL){
        length_string = strlen(tok);      
        commandFromInput[index] = calloc(length_string + 1, sizeof(char));
        for (int i =0; tok[i] != '\0'; i ++){
            commandFromInput[index][i] = tok[i];
        }
        commandFromInput[index][length_string] = '\0';
        
        index ++;

        free(tok);        
    }  
    commandFromInput[index] = NULL;

    free_tokenizer(strOfTokens);
	
    free(input);
	return commandFromInput;   
}


