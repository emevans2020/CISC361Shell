#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include "sh.h"

#define BUFFERSIZE 256

void StringtoArray(char *input, char **cmds);

void StringtoArray(char *input, char **cmds)
{
	char *temp;
	temp = strtok(input, " ");
	if (temp == NULL)
	{

		cmds[0] = malloc(1 * sizeof(char));
		cmds[0][0] = 0;
		return;
	}
	int len = strlen(temp);
	cmds[0] = malloc(sizeof(char) * len + 1);
	strcpy(cmds[0], temp);
	int i = 1;
	while ((temp = strtok(NULL, " ")) != NULL)
	{
		len = strlen(temp);
		cmds[i] = malloc(sizeof(char) * len + 1);
		strcpy(cmds[i], temp);
		i++;
	}
	cmds[i] = NULL;
}

int sh(int argc, char **argv, char **envp)
{
	char *prompt = calloc(PROMPTMAX, sizeof(char));
	char *commandline = calloc(MAX_CANON, sizeof(char));
	char *command, *arg, *commandpath, *p, *pwd, *owd, *cwd;
	char **args = calloc(MAXARGS, sizeof(char *));
	char cmd[64];
	int uid, i, status, argsct, go = 1;
	struct passwd *password_entry;
	char *homedir;
	struct pathelement *pathlist;

	uid = getuid();
	password_entry = getpwuid(uid);   /* get passwd info */
	homedir = password_entry->pw_dir; /* Home directory to start out with*/

	if ((pwd = getcwd(NULL, PATH_MAX + 1)) == NULL)
	{
		perror("getcwd");
		exit(2);
	}

	cwd = calloc(strlen(pwd) + 1, sizeof(char));
	owd = calloc(strlen(pwd) + 1, sizeof(char));
	memcpy(owd, pwd, strlen(pwd));
	memcpy(cwd, pwd, strlen(pwd));
	prompt[0] = ' ';
	prompt[1] = '\0';

	/* Put PATH into a linked list */
	pathlist = get_path();

	while (go)
	{
		/* print your prompt */
		printf("%s%s >>", prompt, pwd);

		/* get command line and process */
		char buf[BUFFERSIZE];
		fgets(buf, BUFFERSIZE, stdin);
		int len = strlen(buf);
		buf[len - 1] = 0;

		StringtoArray(buf, args);

		if (!strcmp(args[0], "exit"))
		{
			printf("Executing built-in %s\n", args[0]);
			go = 0;
		}
		else if (!strcmp(args[0], "which"))
		{
			char *path = which(args[1], pathlist);
			if (args[1] == NULL)
			{
				printf("Which needs an argument\n");
			}
			else
			{
				for (int i = 1; i < MAXARGS; i++)
				{
					if (args[i] != NULL)
					{
						char *path = which(args[i], pathlist);
						if (path)
						{
							// printf("hi");
							printf("%s\n", path);
							free(path);
						}
						else
						{
							printf("%s Could Not Find: %s\n", args[0], args[i]);
						}
					}
				}
			}
		} /* end of which */
		else if (!strcmp(args[0], "where"))
		{
			char *path = where(args[1], pathlist);
			if (args[1] == NULL)
			{
				printf("Where needs an argument\n");
			}
			else
			{
				for (int i = 1; i < MAXARGS; i++)
				{
					if (args[i] != NULL)
					{
						char *path = where(args[i], pathlist);
						if (path)
						{
							printf("%s\n", path);
							free(path);
						}
						else
						{
							printf("%s Could Not Find: %s\n", args[0], args[i]);
						}
					}
				}
			}
		} /* end of where */
		else if (!strcmp(args[0], "pwd"))
		{
			printf("Executing built-in %s\n", args[0]);
			printPWD();
		}

		else if (!strcmp(args[0], "printenv"))
		{ /*prints environment*/
			printf("Executing built-in %s\n", args[0]);
			//if proper amount of arguments
			if (args[0] != NULL && args[1] == NULL)
			{ /*prints whole environment*/
				int i = 0;
				while (envp[i] != NULL) {
					printf("%s\n", envp[i]);
					i++;
				}
			}
			else if (args[1] != NULL && args[2] == NULL)
			{ /*prints specificed env variable*/
				printenv(&args[1]);
			}
			//if passed in more than 2 arguments
			else
			{
				perror("printenv");
				printf("printenv: Too many arguments.\n");
			}
		}

		else if (!strcmp(args[0], "setenv")){
			printf("Executing built-in %s\n", args[0]);
			
			if (args[0] != NULL && args[1] == NULL)
			{ /*prints whole environment if ran with no arguments*/
				int i = 0;
				while (envp[i] != NULL) {
					printf("%s\n", envp[i]);
					i++;
				}
			}
			// ran with one argument set that as an empty environment variable
			else if (args[1] != NULL && args[2] == NULL){
				setEmptyEnv(args[1]);
			}
			// when ran with two arguments the second one is the value of the first
			else if(args[1] != NULL && args[2] != NULL) {
				setValToEnv (args[1],args[2]);
				/*PATH special case, must free path list before setenv on PATH*/
				if(!strcmp(args[1],"PATH")) {
					free(pathlist);
					pathlist = get_path();
				}
				if (strcmp(args[1], "HOME") == 0) {
					homedir = getenv("HOME");
				}
			}
			else { /* when ran with >2 args prints the same error message to stderr that tcsh does */
				perror("setenv");
				printf("setenv: Too many arguments.\n");
			}
		}	

		else if (!strcmp(args[0], "pid"))
		{
			printf("Executing built-in %s\n", args[0]);
			printPid();
		}
		/* check for each built in command and implement */
		// else if (!strcmp(args[0], "list"))
		// {
		// }
		else if (!strcmp(args[0], "prompt"))
		{
			printf("Executing built-in %s\n", args[0]);
			newPrompt(args[1], prompt);
		}
		else /*  else  program to exec */
		{
			//call which to get the absolute path
			char *cmd = which(args[0], pathlist); 		/* find it using which */
			int pid = fork();
			if (pid)   	/* do fork(), execve() and waitpid() */
			{
				free(cmd);
				waitpid(pid, NULL, 0);
			}
			else
			{
				//try to exec the absolute path
				// execve(cmd, args, envp);
				// printf("exec %s\n", args[0]);
				//Run the program.
				if (execve(cmd, args, envp) < 0)
				{
					//If execve() returns a negative value, the program could not be found.
		 			fprintf(stderr, "%s: Command not found.\n", args[0]);
					exit(0);
				}
			}
		}
	}
	return 0;
} /* sh() */

char *which(char *command, struct pathelement *pathlist)
{
	/* loop through pathlist until finding command and return it.  Return
   	NULL when not found. */
	char *result = malloc(BUFFERSIZE);
	while (pathlist)
	{ // WHICH
		sprintf(result, "%s/%s", pathlist->element, command);
		if (access(result, X_OK) == 0)
		{
			return result;
		}
		pathlist = pathlist->next;
	}
	free(result);
	return NULL;
} /* which() */

char *where(char *command, struct pathelement *pathlist)
{
	/* similarly loop through finding all locations of command */
	char *result = malloc(BUFFERSIZE);
	while (pathlist)
	{ // WHICH
		sprintf(result, "%s/%s", pathlist->element, command);
		if (access(result, F_OK) == 0)
		{
			return result;
		}
		pathlist = pathlist->next;
	}
	free(result);
	return NULL;
} /* where() */

void printPWD()
{
	char cwd[BUFFERSIZE];
	getcwd(cwd, sizeof(cwd));
	printf("%s\n", cwd);
} /* printPWD() */

void list(char *dir)
{ /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
	DIR *direct;
	struct dirent *dent;
	direct = opendir(dir);
	if (direct == NULL)
	{
		perror(dir);
	}
	else
	{
		while ((dent = readdir(direct)) != NULL)
		{
			printf("%s\n", dent->d_name);
		}
	}
	closedir(direct);
} /* list() */

void printenv(char **envp)
{
	char **currEnv = envp;
	printf("%s\n", getenv(*currEnv));
} /* printenv() */

void newPrompt(char *command, char *p)
{
	char buffer[BUFFERSIZE];
	int len;
	if (command == NULL)
	{
		command = malloc(sizeof(char) * PROMPTMAX);
		printf("Input new prompt prefix: ");
		if (fgets(buffer, BUFFERSIZE, stdin) != NULL)
		{
			len = (int)strlen(buffer);
			buffer[len - 1] = '\0';
			strcpy(command, buffer);
		}
		strcpy(p, command);
		free(command);
	}
	else
	{
		strcpy(p, command);
	}
} /* newPrompt() */

void printPid()
{
	printf("");
	int pid = getpid();
	printf("%d\n", pid);
} /* printPid() */

/* commands following set the environment */
void setEmptyEnv(char *name) {
	setenv(name,"",1);
}

void setValToEnv(char *arg1, char *arg2) { 
	// command to set environment when provided more than one command
	setenv(arg1,arg2,1);
} /* setValToEnv() */
/* end of commands for set environment */

// void cd(char *path){
// 	chdir(path);
// } /* cd() */