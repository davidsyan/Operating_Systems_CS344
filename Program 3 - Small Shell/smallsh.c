#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

//tonizer deliminators
#define TOKEN_DELIM " \t\r\n\a"

//number of arguments in line
int numArgs = 0;
//if the process is a background process
int isBg = 0;

//read user input
char *readInput();
//tokenize the input
char **tokenizeInput(char *input);
//execute command input
int executeCommand(char **args);
//signal handler
void sig_handler(int status);

int main() {
	//when 'exit' is called, shell ends
	int exitShell = 0;	
	//exit status
	int exitStatus = 0;

	//constantly run program
	do {
		//information entered by user
		char *input = NULL;
		//tokenized arguments from line
		char **args;// = malloc(512*sizeof(char*));
		//file descriptor
		int fileDesc;
		//reset variables
		numArgs = 0;
		isBg = 0;

		//print colon for each line
		printf(": ");

		//get input from user
		input = readInput();		

		//tokenize input string
		args = tokenizeInput(input);	
		
		//check arguments for background, builtin processes, commands, etc
		
		//if command is to be a background process
		//only do something if there is input
		if(numArgs > 0) {
				//strncmp(str1, str2, number of chars)
				if(strncmp(args[numArgs-1], "&", 1) == 0) {
					//make last argument NULL
					args[numArgs-1] = NULL;
					//make background process true
					isBg = 1;
				}		
		
			//printf("%s\n", args[0]);	
			//check for comments or empty commands
			if((strncmp(args[0], "#", 1) == 0)) {// || (args[0] == NULL)) {
				exitStatus = 0;	
			}

			//check for cd
			else if(strcmp(args[0], "cd") == 0) {
				//directory name has been input, move to that directory
				if(args[1]) {
					//if directory does not exist
					if(chdir(args[1]) != 0) {
						printf("No such file or directory\n");
						exitStatus = 1;
					}
				}
				//go to parent directory
				else {
					chdir(getenv("HOME"));
					exitStatus = 0;
				}
			}

			//check for status
			else if(strcmp(args[0], "status") == 0) {
				printf("Exit Status %d\n", exitStatus);
				exitStatus = 0;
			}	

			//check for file redirection
			else if(numArgs == 3 && ((strcmp(args[1], ">") == 0) || 
									(strcmp(args[1], "<") == 0))) {
				//assign stdin and stdout variables
				int stdin = dup(0);
				int stdout = dup(1);
				
				//if writing to file
				if(strcmp(args[1], ">") == 0) {
					//open file
					fileDesc = open(args[2], O_WRONLY|O_CREAT|O_TRUNC, 0644);
					//if file does not exist
					if(fileDesc == -1) {
						printf("No such file or directory\n");
						exitStatus = 1;
					}	
					//file does exist
					else {
						//assign stdout to this file
						dup2(fileDesc, 1);
						//clear out everything but command
						args[1] = NULL;
						//close the file
						close(fileDesc);
						//execute command
						exitStatus = executeCommand(args);
					}
				}
				//if reading from file
				else if(strcmp(args[1], "<") == 0) {
					fileDesc = open(args[2], O_RDONLY);
					if(fileDesc == -1) {
						printf("No such file or directory\n");
						exitStatus = 1;
					}
					else {
						dup2(fileDesc, 0);
						args[1] = NULL;
						close(fileDesc);
						exitStatus = executeCommand(args);
					}
				}	

				//restore stdin and stdout
				dup2(stdin, 0);
				close(stdin);
				dup2(stdout, 1);
				close(stdout);
			}

			//check for exit command
			else if(strcmp(args[0], "exit") == 0) {
				exitShell = 1;
				exitStatus = 0;	
			}

			//check for all other commands (ls is handled here)
			else {
				exitStatus = executeCommand(args);
			}		

			//free memory
			free(args);
			free(input);
		}	
	}while(!exitShell);

	return 0;
}

char *readInput() {
	char *input = NULL;
	size_t size = 0;
	
	//get input
	getline(&input, &size, stdin);
	
	return input;
}

char **tokenizeInput(char *input) {
	//max number of arguments
	int size = 512;
	//token number
	int num = 0;
	//array of each token
	char **tokenArr = malloc(size*sizeof(char*));
	//each individual token
	char *token;

	//tokenize string
	token = strtok(input, TOKEN_DELIM);
	//place each token in token array
	while(token != NULL) {
		//place token in array
		tokenArr[num] = token;
		++num;
		++numArgs;
		
		//tokenize string
		token = strtok(NULL, TOKEN_DELIM);
	}
	//last array position is NULL
	tokenArr[num] = NULL;
	
	return tokenArr;
}

int executeCommand(char **args) {
	pid_t pid;
	pid_t wpid;
	int status = 0;
	int exitStatus = 0;
	
	struct sigaction sig;
	sig.sa_handler = sig_handler;
	sigemptyset(&sig.sa_mask);
	sig.sa_flags = 0;
	sigaction(SIGINT, &sig, NULL);
		
	//create child process and store pid for the parent
	pid = fork();

	//if child process 	
	if(pid == 0) {
		//execute commands
		//child process executes new program
		if(execvp(args[0], args) == -1) {
			printf("No such file or directory\n");
			//exit the chile process
			exit(1);
		}
		if(isBg == 0) {
			sig_handler(status);
			sigaction(SIGINT, &sig, NULL);
		}
	}
	//if there was an error during the forking
	else if(pid < 0) {
		perror("smallsh");
	}
	//for the parent process
	//parent process waits for the child process to be completed
	else {
		do {
			if(isBg == 0) {
				wpid = waitpid(pid, &status, WUNTRACED);
			}
			else if(isBg == 1) {
				wpid = waitpid(-1, &status, WNOHANG);
				while(wpid > 0) {
					wpid = waitpid(-1, &status, WNOHANG);
				}
			}
		//wait for the child to be killed by a signal or exit
		}while(!WIFSIGNALED(status) && !WIFEXITED(status));
	}

	//parent uses child process to set exitStatus
	if(status != 0 || WIFSIGNALED(status)) {
		printf("Background pid %d is done: ", wpid);
		sig_handler(status);
		exitStatus = 1;
	}

	//background process completed
	if(isBg == 1) {
		printf("Background PID: %d\n", pid);
		printf("Exit status: %d\n", exitStatus);
	}
	
	return exitStatus;
}

void sig_handler(int status) {
	printf("Terminated by signal %d\n", status);
}
