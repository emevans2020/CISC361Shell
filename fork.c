#include <unistd.h>
#include <stdio.h>

int main() 
{
  pid_t pid;
	    
  if((pid = fork()) < 0) {
    puts("ERROR");
  }
  else if(pid == 0) {
    puts("Hello");
  }
  else {
    waitpid(pid, NULL, 0);
    puts("Goodbye");
  }
}
