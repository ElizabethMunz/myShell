#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

//function declarations
char* readline();
char** parse(char* input);
int execute(char** args);
//builtin functions
int quit(char** args);
int cd(char** args);
int clear(char** args);
int dir(char** args);
int environ(char** args);
int echo(char** args);
int help(char** args);

#define BUFSIZE  1024

//list of builtin command keywords
char* builtins[] = {"quit", "cd", "clear", "dir", "environ", "echo", "help"};
int (*builtinFN[])(char **) = {&quit, &cd, &clear, &dir, &environ, &echo, &help};

// The main function: infinite loop that is the shell
void main(int argc, char*argv[]) {
  //a will be the exit code/return value of any executed program or function
  int a = 1;
  while(a != 0) {
    //get current directory to print in prompt
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    //print prompt
    printf("$myShell:~%s>", cwd);
    char* input = readline();
    char** inputArray = parse(input);
    a = execute(inputArray);
  }
}

//read user input until user hits enter, then return that input string
char* readline() {
  char* input = malloc(sizeof(char) * BUFSIZE);
  int pos = 0;
  char c;
  while((c = getchar()) != '\n' & c != EOF) {
    input[pos] = c;
    pos++;
  }
  input[pos] = '\0';
  return input;
  //TODO: handle if input is longer than BUFSIZE, need to allocate more space to input
}

//split user input into an array of char* based on spaces
char** parse(char* input) {
  int i = 0;
  char** arrayArgs = malloc(sizeof(char) * BUFSIZE);
  const char* space = " ";
  char* arg = strtok(input, space);

  while(arg != NULL) {
    arrayArgs[i] = arg;
    //this printf just for testing purposes (to confirm args were split correctly)
    //printf("args %d = %s\n", i, arrayArgs[i]);
    i++;
    arg = strtok(NULL, space);
  }
  arrayArgs[i] = NULL;
  return arrayArgs;
}

//based on values of args array, execute appropriate function (either builtin or external)
int execute(char** args) {

  int ret = 1; //returns 0 on quit, 1 otherwise
  int isBuiltin = 0;
  int exitStat;

  pid_t pid;
  pid = fork();
  if(pid < 0) {
    printf("Error forking\n");
  }
  else if(pid == 0) {
    //in child
    //TODO: check if we need to redirect I/0 or run in background

    //check for builtin, then run matching fn if it is
    int j;
    for(j = 0; j < 7; j++) { //j = number of builtin functions there are
      if(strcmp(args[0], builtins[j]) == 0) {
        //set return value
        ret = (builtinFN[j])(args);
        isBuiltin = 1;
      }
    }
    //if we didn't find a builtin fn, try to exec a file
    if(isBuiltin == 0) {
      execvp(args[0], args) || 1;
      //there was no file to exec, so tell the user they're wrong
      printf("Command or executable not recognized.\n");
    }
    //exit with return value
    exit(ret);
  }
  else {
    //in parent
    waitpid(pid, &exitStat, 0);
    //printf("exit status of child: %d\n", exitStat);
    return exitStat;
  }
}

//---------------------builtin functions------------------------------

//quit shell
int quit(char** args) {
  return 0;
}

int cd(char** args) {
  if(chdir(args[1]) != 0) {
    printf("No directory '%s' found.\n", args[1]);
  }
  return 1;
}

int clear(char** args) {
  int screenHeight = 100; //hardcode a number for screenHeight b/c there's no portable way to get actual window height in C
  int i;
  for(i = 0; i < screenHeight; i++) {
    printf("\n");
  }
  return 1;
}

int dir(char** args) {
  DIR *dir;
  struct dirent *dirContents;
  if(args[1] == NULL) {
    //if no argument specified, get current directory contents
    dir = opendir("./");
  }
  else {
    //check if specified directory exists, open it
    if((dir = opendir(args[1])) == NULL) {
      printf("No such directory %s.\n", args[1]);
      return 1;
    }
  }
  while((dirContents = readdir(dir)) != NULL) {
    printf(" %s ", dirContents->d_name);
  }
  printf("\n");
  closedir(dir);
  return 1;
}

int environ(char** args) {
  const char* env = getenv("PATH");
  if(env != NULL) {
    printf("%s\n", env);
  }
  else {
    printf("getenv() returned NULL");
  }
  return 1;
}

int echo(char** args) {
  printf("%s\n", args[1]);
  return 1;
}

//print out explanation of how to use the shell
int help(char** args) {
  printf("----------MYSHELL HELP----------\n How to use:  help [command]  to learn about built-in command\n--------------------------------\n");
  if(args[1] == NULL) {
    printf(" Commands:\n  cd\n  clear\n  dir\n  echo\n  environ\n  help\n  quit\n");
    printf(" This shell also accepts running executable files. Type the relative path of the executable and hit enter to run it.\n");
  }
  else {
    //give help for specific command
    int j;
    for(j = 0; j < 7; j++) { //j = number of builtin functions there are
      if(strcmp(args[1], builtins[j]) == 0) {
        printf(" How to use: %s\n", builtins[j]);
        break;
      }
    }
  }
  printf("--------------------------------\n");
  return 1;
}

