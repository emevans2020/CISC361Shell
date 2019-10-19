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
			printf ("Exiting Now!!! \n");
			go=0;
		}
		else if (!strcmp(args[0], "which"))
		{
			char *path = which(args[1], pathlist);
			if (args[1] == NULL)
			{
				printf("Which needs an argument\n");
			}
			// printf ("%s",path);
			if (args[1] != NULL)
			{
				if (path)
				{
					// printf("hi");
					printf("%s\n", path);
					free(path);
				}
				else
				{
					printf("%s Could Not Find: %s\n", args[0],args[1]);
				}
			}
		}
		else if (!strcmp(args[0], "where"))
		{
			char *path = which(args[1], pathlist);
			if (args[1] == NULL)
			{
				printf("Where needs an argument\n");
			}
			// printf ("%s",path);
			if (args[1] != NULL)
			{
				if (path)
				{
					// printf("hi");
					printf("%s\n", path);
					free(path);
				}
				else
				{
					printf("%s Could Not Find: %s\n", args[0],args[1]);
				}
			}
		}
		else if (!strcmp(args[0], "pwd")) {
			printPWD();
		}

		else if (!strcmp(args[0], "pid")) {
			printPid();
		}
		/* check for each built in command and implement */
		else if (!strcmp(args[0], "list")) {
			
		}
		else if(!strcmp(args[0],"prompt")){
			newPrompt(args[1],prompt);
		}
		else
		{
			//call which to get the absolute path
			char *cmd = which(args[0], pathlist);
			int pid = fork();
			if (pid)
			{
				free(cmd);
				waitpid(pid, NULL, 0);
			}
			else
			{
				//try to exec the absolute path
				// execve(cmd, args, envp);
				// printf("exec %s\n", args[0]);
				exit(0);
			}
		}
		/*  else  program to exec */
		/* find it */
		/* do fork(), execve() and waitpid() */
		/* else */
		/* fprintf(stderr, "%s: Command not found.\n", args[0]); */
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
	{ // WHERE
		sprintf(result, "%s/%s", pathlist->element, command);
		if (access(command, F_OK) == 0)
			printf("[%s]\n", command);
		pathlist = pathlist->next;
	}
	printf(command, ": Command not found.");
	return NULL;
} /* where() */

void printPWD(){
	char cwd[BUFFERSIZE];
	getcwd(cwd,sizeof(cwd));
	printf("%s\n",cwd);
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

} /* printenv() */

void newPrompt(char *command, char *p) 
{
  char buffer[BUFFERSIZE];
  int len;
  if (command == NULL) 
  {
    command = malloc(sizeof(char) * PROMPTMAX);
    printf("Input new prompt prefix: ");
    if (fgets(buffer, BUFFERSIZE, stdin) != NULL) {
    len = (int) strlen(buffer);
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

void printPid(){
	printf("");
	int pid = getpid();
	printf("%d\n", pid);
} /* printPid() */