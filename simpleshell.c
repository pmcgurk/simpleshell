/************************************************************************************
*		
*	Synopsis:
*	This program is a simple C shell for Linux
*	
*	File: simpleshell.c
*
*	Authors: Paul McGurk, Jordan Melville.
*
*	All code, unless specified, is of our own creation.
*
* ***********************************************************************************/

#define VERSION "simpleshell v1.0. Last update: 13/04/2014\n\n"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define terminate 0
#define maxHistSize 20
#define MAX_INPUT 512

char *history[maxHistSize];
int historySize = 0;
char *aliases[20][2];
int aliasesSize = 0;
char *tokens[50];
char *path;

void setup() {
	/* attempt to open file */
	FILE *rf = fopen("history.txt", "rb");
	/* if there is no file, return */
	if (rf) {
		fread(history, sizeof(char), historySize, rf);
	}
}

void addHist() {
	int i;
	/* If historySize if not at the limit of History Array */
	if (strcmp(tokens[0], "history") != 0) {
	   	if (historySize < maxHistSize) {
	        	history[historySize++] = strdup(*tokens);
	   	} 
		/* else move all elements up and add */
		else {
			for (i = 1; i < maxHistSize; i++) {
				history[i - 1] = history[i];
				}
				history[maxHistSize - 1] = strdup(*tokens);
		}
	}
}

void  invokeHist() {
	int num = historySize - 1, i , hist;
	/* check if is a digit and is within historySize */
	if ((tokens[0][1] - '0') > historySize) {
		printf("History does not go back that far.\n");
	}
	else {
		/* call last command */
		if (!strncmp(tokens[0], "!!", 2)) {
			num = historySize - 1;
		}
		else if (!isdigit(tokens[0][1])) {
			printf("Please enter a digit!\n");
		}
		/* call command x in history */
		else if (isdigit(tokens[0][2])) {
			hist = (tokens[0][2] - '0') + ((tokens[0][1] - '0') * 10);
			num = historySize - 1 - hist;
		} else {
			hist = (tokens[0][1] - '0');
			num = historySize - 1 - hist;
		}
		tokens[0] = strdup(history[num]);
	}		
}

printHist() {
	int i = 0;
	/* Print function */
	for (i = 0; i < historySize; i++) {
		if (historySize - i - 1 != 0) {
			printf("!%d for %s\n", historySize - i - 1, history[i]);
		}
		else {
				printf("!! for %s\n", history[i]);
		}
	}
}
/* Handles the input if it is to do with aliases */

void aliasesControl(char *cmd[]) {
	int i;
	if (cmd[1] == NULL) {
		if (aliasesSize == 0) {
			printf("Aliases are empty.\n");
		}
		else {
			for (i = 0; i < aliasesSize; i++) {
				printf("%s is aliase for %s\n", aliases[i][0], aliases[i][1]);
			}
		}
	}
	else {
		addAliases();
	}
}

void addAliases() {
	aliases[aliasesSize][0] = strdup(tokens[1]);
	aliases[aliasesSize][1] = strdup(tokens[2]);
	printf("%s assigned to %s.\n", aliases[aliasesSize][0], aliases[aliasesSize][1]);
	aliasesSize++;
}

/* invoke aliases  */
void invokeAlias(char *cmd[]) {
	int i = 0;
	/************************************************
	*	go through the array of aliases and			*
	*	change the token[0] (the command) to the	*
	*	alias command, were applicable.				*
	*												*
	************************************************/
	for (i = 0; i < aliasesSize; i++) {
		if (!strcmp(cmd[0], aliases[i][0])) {
			tokens[0] = strdup(aliases[i][1]);
		}
	}
}

/* tokenize input */
int tokenizedInput(char input[], char *tokens[]) {
	int i = 0, j;
	char *token;
	int len;
	if (fgets(input, 512, stdin) != 0) {
		len = strlen(input);
	}
	else {
		exit(EXIT_SUCCESS);
	}
	token = strtok(input, " <>|\t\n");
	while (token != NULL)
	{
		tokens[i++] = token;
		token = strtok(NULL, " <>|\t\n");
	}
	tokens[i] = NULL;
	return i;
}

/* print working directory */
int pwd() {
	char buffer[MAX_INPUT];
	if (tokens[1] != NULL) {
		printf("pwd does not take in any variables.\n");
	}
	else {
		getcwd(buffer, MAX_INPUT);
		printf("%s\n", buffer);
	}
	return 0;
}

/* change directory */
int cd() {
	char *temp;
	if (tokens[1] == NULL) {
		printf("HOME\n");
		temp = getenv("HOME");
		chdir(temp);
	}
	else if (chdir(tokens[1]) == -1) {
		perror("cd");
	}
	else if (tokens[2] != NULL) {
		printf("The command cd should only take 1 or 0 variables.\n");
	}
}

void setpath() {
	if (tokens[1] != NULL && tokens[2] == NULL) {
		setenv("PATH", tokens[1], 1);
		printf("The path has been set to %s\n", getenv("PATH"));
	}
	else if (tokens[2] != NULL) {
		printf("The path was not set as too many variables were entered.\n");
	}
	else {
		printf("The path was not set as no path was specified.\n");
	}
}

void getpath() {
	if (tokens[1] != NULL) {
		printf("Cannot get path, as too many variables were entered.\n");
	}
	else {
		printf("The path is %s\n", getenv("PATH"));
	}
}

void exterCmd() {
	// forks for the new execvp command
	pid_t pid = fork();
	if (pid < 0) {
		fprintf(stderr, "The fork has failed");
		exit(-1);
	}
	else if (pid == 0) {
		if (execvp(tokens[0], tokens) == -1) {
			printf("Command \"%s\" not found.\n", tokens[0]);
			exit(0);
		}
	}
	else {
		wait(NULL);
		printf("Child process complete\n");
	}
}

void exitCmd() {
	// Opens the history file
	FILE *f = fopen("history.txt", "wb");
	// Writes the contents of the array
	fwrite(history, sizeof(char), historySize, f);
	fclose(f);
	// Sets the Path to the home path
	setenv("PATH", path, 1);
	printf("The path is %s\n", getenv("PATH"));
	exit(EXIT_SUCCESS);
}

void handleInput(char *tokens[]) {
	if (tokens[0] != NULL) {
		invokeAlias(tokens);
		if (strncmp(tokens[0], "!", 1) == 0) {
			invokeHist(tokens);
		}
		else {
			addHist();
		}
		if (strcmp(tokens[0], "history") == 0) {
			printHist();
		}
		else if (strcmp(tokens[0], "alias") == 0) {
			aliasesControl(tokens);
		}
		else if (strcmp(tokens[0], "exit") == 0) {
			exitCmd();
		}
		else if (strcmp(tokens[0], "pwd") == 0) {
			pwd();
		}
		else if (strcmp(tokens[0], "cd") == 0) {
			cd();
		}
		else if (strcmp(tokens[0], "setpath") == 0) {
			setpath();
		}
		else if (strcmp(tokens[0], "getpath") == 0) {
			getpath();
		}
		else {
			exterCmd();
		}
	}
}

int main() {
	char input[512];
	path = getenv("PATH");
	char *homedir = getenv("HOME");
	int tokenCounter = 0;
	printf(VERSION);
	setup();
	while (terminate == 0) {
		printf("%s>$ ", homedir);
		// tokenises the input and assigns it to tokens, returns size
		tokenCounter = tokenizedInput(input, tokens);
		// Handles the tokenized input
		handleInput(tokens);
	}
}

