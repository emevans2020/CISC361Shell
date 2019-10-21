#include "get_path.h"

int pid;
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
char *where(char *command, struct pathelement *pathlist);
void printPWD();
void list ( char *dir );
void printenv(char **envp);
void newPrompt(char *command, char *p);
void printPid();
void setEmptyEnv(char *name);
void setValToEnv(char *arg1, char *arg2);

#define PROMPTMAX 32
#define MAXARGS 10
