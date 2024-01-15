#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

//#define DEBUG
#define INPUT_SIZE 1024

pid_t childPid = 0;

void executeShell(int timeout);

void writeToStdout(char *text);

void alarmHandler(int sig);

void sigintHandler(int sig);

char *getCommandFromInput();

void registerSignalHandlers();

void killChildProcess();

int main(int argc, char **argv) {
    registerSignalHandlers();

    int timeout = 0;
    if (argc == 2) {
        timeout = atoi(argv[1]);
    }

    if (timeout < 0) {
        writeToStdout("Invalid input detected. Ignoring timeout value.\n");
        timeout = 0;
    }

	
    while (1) {
	    	
   	alarm(0);         //sets alarm for user defined seconds 
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
     if (sig == SIGALRM) {
	if (childPid != 0) {
       		killChildProcess();
       		writeToStdout("Bwahaha ... tonight I dine on turtle soup\n");
	}
    }
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
    if(signal(SIGALRM, alarmHandler) == SIG_ERR){ 
	    //perror("Unable to catch SIGINT\n"); 
	    perror("Error in signal");	    
	    exit(EXIT_FAILURE);
    }
}

/* Prints the shell prompt and waits for input from user.
 * Takes timeout as an argument and starts an alarm of that timeout period
 * if there is a valid command. It then creates a child process which
 * executes the command with its arguments.
 *
 * The parent process waits for the child. On unsuccessful completion,
 * it exits the shell. */
void executeShell(int timeout) {
    char *command;
    int status;
    char minishell[] = "penn-shredder# ";	
    writeToStdout(minishell);
    command = getCommandFromInput();
		
	//identify if user entered blank spaces and then enter key or enter key alone, and if that is the case loop again to execute shell in main
	if (strlen(command) == 0){		
		free(command);
		return;} 
    alarm(timeout);
    if (command != NULL) {
	    //printf("line110 %d", atoi(command));
        childPid = fork();

        if (childPid < 0) {
            perror("Error in creating child process");
            free(command);
            exit(EXIT_FAILURE);
        }

        if (childPid == 0) {
            char *const envVariables[] = {NULL};
            char *const args[] = {command, NULL};
            if (execve(command, args, envVariables) == -1) {
                perror("Error in execve");
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
	
            childPid = 0;
            free(command);
	} 
    } else (printf("the command is null"));
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
char *getCommandFromInput() {
		char *input = (char*) calloc(INPUT_SIZE, sizeof(char));
	//char* input;        //uncomment top one and comment out this one
	
	//check for error in calloc
	if (input == NULL){
		perror("could not allocate memory");
		free(input);
	}
	
	int numbytesread;
	char text[INPUT_SIZE];
	int index =0;
	
	
	//read from standard input into input char string upto 1024 bytes
	numbytesread = read(STDIN_FILENO, input, INPUT_SIZE);
	
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
	for(int i=0; i <INPUT_SIZE; i++){							       
		if(!isspace(input[i])){
		text[index] = input[i]; 
		if(text[index] =='\n'){break;}
		index++;}
		}
		text[index] ='\0';
	
	strcpy(input, text);	
	
	//if(write(STDOUT_FILENO, input, index)== -1){
		//free(input);
		//perror("Error in write");
		//exit(EXIT_FAILURE);
	//}
	
	#ifdef DEBUG
	printf("input string is %s\n", input);
	printf("text string is %s\n", text);	
	#endif
	
	return input;
   // return "/bin/ls";
}
