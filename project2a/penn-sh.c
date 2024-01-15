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
	///below if for part b
	//if (childPid != 0) {
       // killChildProcess();
       //writeToStdout("    ");
//}
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

	char **command = getCommandFromInput();

    #ifdef DEBUG
	printf("input string first in execshell is %s\n", command[0]);
    printf("input string second in execshell is %s\n", command[1]);
    printf("input string third in execshell is %s\n", command[2]);	
	#endif
    
   // if(command[0] == NULL){
       // printf("121 is true");
//return;}

    if (command[0] != NULL) {	  
        childPid = fork();

        if (childPid < 0) {
            perror("Error in creating child process");
           // free(command);
            exit(EXIT_FAILURE);
        }

        if (childPid == 0) {
           //char *const envVariables[] = {NULL};
           //initialize the index of command array and args array
           int index = 0;
           int argsIndex = 0;
//printf("args index on line 135 is %d\n", argsIndex);
           while(command[index] !=NULL){

            if(strchr(command[index] ,'>' )!= NULL) {
                int outRedirectCt = getRedirectionCount(command, index);
                if( argsIndex == 0){ argsIndex = index;}
//printf("line 140 check for argindex %d\n",argsIndex);
                if (outRedirectCt >1){
                    perror("Invalid number of redirections");
                    return;
                }              
                redirectionSTDOUTtoFile(command[index+1]);
            }

            if(strchr(command[index] ,'<' )!= NULL){
                int outRedirectCt = getRedirectionCount(command, index);
                if( argsIndex == 0){ argsIndex = index;}
                if (outRedirectCt >1){
                    perror("Invalid number of redirections");
                    return;
                }
                redirectionSTDINtoFile(command[index+1]);
 //printf("line156 check for argindex %d\n",argsIndex);               
            }
            index++;

//printf("lien 159 args index is %d\n", argsIndex);
//printf("line 160 array total index is %d\n", index);
           }
//printf("line 162 args index is %d\n", argsIndex);
//printf("line 163 array total index is %d\n", index);
           char * args[index] ;

           if (argsIndex != 0){
           for(int i=0; i< argsIndex; i++){
            args[i] = command[i];
//printf("line 168 args for index %d is %s\n",i, args[i]);
           }
           args[argsIndex] = '\0';
//printf("line 171 args for last index %d is %s\n",argsIndex, args[argsIndex]);          
           }

           else {
            for (int j =0; j<index; j++){
                args[j] = command[j];
//printf("line 176 args for index %d is %s\n",j, args[j]);
            }
            args[index] = '\0';
// printf("line 179 args for last index %d is %s\n",index, args[index]); 
           }
          
            if (execvp(command[0], args) == -1) {
                perror("Error in execvp");
               // free(command);
                exit(EXIT_FAILURE);
            }
        } else {
            do {
                if (wait(&status) == -1) {
                    perror("Error in child process termination");
                   // free(command);
                    exit(EXIT_FAILURE);
                }
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
            childPid = 0;
            //free(command);
        
    } else (printf("the command is null"));
}

void redirectionSTDOUTtoFile(char* filename){
//printf("line 204 filename %s\n", filename);
  int new_stdout = open(filename, O_WRONLY | O_CREAT, 0644);
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
  char *commandFromInput[] = {NULL};
char **getCommandFromInput() {
    //char **commandFromInput = {NULL};
   // char ** dblptr;
    TOKENIZER  *strOfTokens;
	
   // char **commandFromInput = {NULL};
   // *commandFromInput = (char*) calloc(INPUT_SIZE, sizeof(char));


    char *tok;
    //char *input = NULL;
    char *input = (char*) calloc(INPUT_SIZE, sizeof(char));

	if (input == NULL){
		perror("could not allocate memory");
		free(input);
	}
	
	int numbytesread;
	
	int index =0;
		
	//read from standard input into input char string upto 1024 bytes
	numbytesread = read(STDIN_FILENO, input, INPUT_SIZE);

	#ifdef DEBUG
    printf("numbytesread is  %d\n", numbytesread);
    //printf("input string is %s\n", input);
    #endif

	//if error occured in reading then exit with an error message
	if (numbytesread == -1){
		free(input);
		perror("Error in read");
		exit(EXIT_FAILURE);
	}
	
	//if ^D is entered, exit the entire program successfully
	if (numbytesread == 0){ 
		free(input);
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
      //  printf("input string token is %s\n", tok);
       
        commandFromInput[index] = tok;
        //strcpy(commandFromInput[index], tok);
       // printf("input string token from commandarray at index %d is %s\n", index, commandFromInput[index]);
       // free(tok);
        index ++;
    } 
   //free_tokenizer(strOfTokens);
    
    commandFromInput[index] = NULL;
	
	#ifdef DEBUG
	//printf("input string 2   is %s\n", input);
	//printf("text string is %s\n", text);	
   // for (int i=0; i <3; i++){
      //  printf("%s\n",commandFromInput[i]);
    
	#endif
	
    //dblptr = commandFromInput;
   // printf("input array first entry is %s\n", dblptr[0]);
   // printf("input array third entry is %s\n", dblptr[2]);
    free(input);
	return commandFromInput;
   
}


